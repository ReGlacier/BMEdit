#include <GameLib/MAT/MATReader.h>
#include <GameLib/MAT/MATSubClass.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>


namespace gamelib::mat
{
	MATReader::MATReader() = default;

	bool MATReader::parse(const uint8_t *pMatBuffer, size_t iMatBufferSize)
	{
		// https://github.com/glacier-modding/io_scene_blood_money/blob/libraries/BMExport/src/mati.cpp#L148
		// https://github.com/glacier-modding/io_scene_blood_money/blob/libraries/BMExport/src/mati.h
		// see sub_4930C0
		// see sub_497820
		// see sub_471D70
		ZBio::ZBinaryReader::BinaryReader matReader { reinterpret_cast<const char*>(pMatBuffer), static_cast<int64_t>(iMatBufferSize) };

		// Read header
		MATHeader::deserialize(m_header, &matReader);

		// Collect classes
		matReader.seek(m_header.classListOffset);
		if (!collectMaterialClasses(&matReader))
		{
			return false;
		}

		// Collect instances
		matReader.seek(m_header.instancesListOffset);
		if (!collectMaterialInstances(&matReader))
		{
			return false;
		}

		// Save constant table
		// TODO: Copy const table

		return true;
	}

	bool MATReader::collectMaterialClasses(ZBio::ZBinaryReader::BinaryReader* matReader)
	{
		static constexpr size_t kMaxClassCount = 0x20;

		// Clear class list
		m_classes.clear();

		for (size_t classIndex = 0; classIndex < kMaxClassCount; ++classIndex)
		{
			const auto classDeclOffset = matReader->read<uint32_t, ZBio::Endianness::LE>();
			if (classDeclOffset == 0)
				continue; // null reference (skipped)

			// Save position to seek back
			ZBioSeekGuard guard(matReader);

			// Continue work (we will seek back on end of iteration)
			matReader->seek(classDeclOffset);

			// Read class description
			MATClassDescription classDescription;
			MATClassDescription::deserialize(classDescription, matReader);

			if (classDescription.classDeclarationOffset < 0x10)
			{
				assert(false && "Bad class declaration offset");
				return false;
			}

			// Jump to class decl
			matReader->seek(classDescription.classDeclarationOffset);

			// Read header decl
			MATPropertyEntry classDeclEntry;
			MATPropertyEntry::deserialize(classDeclEntry, matReader);

			// Check that entry is valid
			if (classDeclEntry.kind != MATPropertyKind::PK_CLASS)
			{
				assert(false && "Expected to have class here");
				return false;
			}

			if (classDeclEntry.valueType != MATValueType::PT_LIST)
			{
				assert(false && "Expected to have list here");
				return false;
			}

			// Jump to first entry and iterate over entries
			matReader->seek(classDeclEntry.reference);

			std::vector<MATPropertyEntry> properties;
			std::vector<MATSubClass> subclasses;
			subclasses.reserve(classDeclEntry.containerCapacity);
			properties.resize(classDeclEntry.containerCapacity);

			std::string className;

			for (int i = 0; i < classDeclEntry.containerCapacity; i++)
			{
				MATPropertyEntry::deserialize(properties[i], matReader);

				if (properties[i].kind == MATPropertyKind::PK_NAME)
				{
					assert(className.empty() && "Possible bug: class name override detected!");

					// Maybe class name override?
					ZBioSeekGuard readNameGuard { matReader };

					matReader->seek(properties[i].reference);
					className = matReader->readCString();
				}
				else if (properties[i].kind == MATPropertyKind::PK_SUB_CLASS)
				{
					// Build subclass from entry
					ZBioSeekGuard buildSubClassGuard { matReader };
					matReader->seek(properties[i].reference);

					subclasses.emplace_back(MATSubClass::makeFromStream(matReader, properties[i].containerCapacity));
				}
			}

			if (className.empty())
			{
				assert(false && "Expected at least 1 PK_NAME property, but no one presented.");
				return false;
			}

			m_classes.emplace_back(
			    std::move(className),
			    std::move(classDescription.parentClass),
			    std::move(properties),
			    std::move(subclasses)
			);
		}

		return true;
	}

	bool MATReader::collectMaterialInstances(ZBio::ZBinaryReader::BinaryReader* matReader)
	{
		static constexpr size_t kMaxMaterialInstancesCount = 0x800;

		for (size_t instanceIndex = 0; instanceIndex < kMaxMaterialInstancesCount; ++instanceIndex)
		{
			const auto instanceOffset = matReader->read<uint32_t, ZBio::Endianness::LE>();
			if (instanceOffset == 0)
				continue; // skip null reference

			// Save position in guard
			ZBioSeekGuard guard { matReader };

			// Jump to offset
			matReader->seek(instanceOffset);

			MATInstanceDescription instanceDescription;
			MATInstanceDescription::deserialize(instanceDescription, matReader);

			if (instanceDescription.instanceDeclarationOffset < 0x10)
			{
				assert(false && "Bad instance declaration offset");
				return false;
			}

			// Seek to decl position
			matReader->seek(instanceDescription.instanceDeclarationOffset);

			// Read instance definition property
			MATPropertyEntry instanceDeclarationProperty;
			MATPropertyEntry::deserialize(instanceDeclarationProperty, matReader);

			if (instanceDeclarationProperty.kind != MATPropertyKind::PK_INSTANCE)
			{
				assert(false && "Instance expected!");
				return false;
			}

			if (instanceDeclarationProperty.valueType != MATValueType::PT_LIST)
			{
				assert(false && "List in instance expected!");
				return false;
			}

			// Jump to instance
			matReader->seek(instanceDeclarationProperty.reference);

			// Read properties
			std::vector<MATPropertyEntry> properties;
			std::vector<MATBind> binders;
			binders.reserve(instanceDeclarationProperty.containerCapacity);
			properties.resize(instanceDeclarationProperty.containerCapacity);

			std::string instanceName;

			for (int i = 0; i < instanceDeclarationProperty.containerCapacity; i++)
			{
				MATPropertyEntry::deserialize(properties[i], matReader);

				if (properties[i].kind == MATPropertyKind::PK_NAME)
				{
					assert(instanceName.empty() && "Possible bug: instance name override detected!");

					ZBioSeekGuard seekNameGuard { matReader };

					matReader->seek(properties[i].reference);
					instanceName = matReader->readCString();
				}
				else if (properties[i].kind == MATPropertyKind::PK_BIND)
				{
					ZBioSeekGuard seekBinderGuard { matReader };

					matReader->seek(properties[i].reference);

					binders.emplace_back(MATBind::makeFromStream(matReader, static_cast<int>(properties[i].containerCapacity)));
				}
			}

			if (instanceName.empty())
			{
				assert(false && "Expected to have instance name, but it's not presented!");
				return false;
			}

			// Save instance
			m_instances.emplace_back(
			    std::move(instanceName),
			    std::move(instanceDescription.instanceParentClassName),
			    std::move(properties),
			    std::move(binders)
			);
		}

		return true;
	}
}