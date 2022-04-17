#include <GameLib/GMS/GMSEntries.h>
#include <GameLib/BinaryReaderSeekScope.h>
#include <ZBinaryReader.hpp>


namespace gamelib::gms
{
	GMSEntries::GMSEntries() = default;

	const std::vector<GMSGeomEntity> &GMSEntries::getGeomEntities() const
	{
		return m_entities;
	}

	void GMSEntries::deserialize(GMSEntries &entries, ZBio::ZBinaryReader::BinaryReader *gmsFileReader, ZBio::ZBinaryReader::BinaryReader *bufFileReader)
	{
		const auto geomTableOffset = gmsFileReader->tell();

		struct GeomDescription
		{
			uint32_t declarationOffset { 0 };
			uint32_t unk4 { 0 };
		};

		const auto entitiesCount = gmsFileReader->read<uint32_t, ZBio::Endianness::LE>();
		entries.m_entities.reserve(entitiesCount + 1); // +1 for ROOT entity, it's not declared in GMS but must be allocated!

		// Allocate ROOT (ZROOM) entity
		{
			auto &root = entries.m_entities.emplace_back();
			root.m_name = "ROOT";
			root.m_typeId = 0x100021; //NOTE: Maybe we should use some sort of constant here? Or take this value from types database?
			//NOTE: Maybe something else should be declared here?
		}

		// Allocate other GEOMs
		for (int entId = 0; entId < entitiesCount; ++entId)
		{
			BinaryReaderSeekScope entityScope { gmsFileReader };

			constexpr int kEntrySize = 8;

			const auto geomAddress = geomTableOffset + 4 + (entId * kEntrySize);
			gmsFileReader->seek(geomAddress);

			GeomDescription desc {};
			desc.declarationOffset = gmsFileReader->read<uint32_t, ZBio::Endianness::LE>();
			desc.unk4 = gmsFileReader->read<uint32_t, ZBio::Endianness::LE>();

			auto &currentGeomEntry = entries.m_entities.emplace_back();

			{
				BinaryReaderSeekScope geomScope { gmsFileReader };

				/**
				 * Declaration offset actually contains 2 values (NOTE: Maybe three)
				 *
				 *   (1)       (2)
				 * [ FF ] [ FF FF FF ]
				 *
				 *  (1) - Depth(?) information
				 *  (2) - Offset information
				 *
				 * Real offset could be calculated via multiply value (2) by 4
				 * Read depth information could be found via offset (1) by 1 bit to right
				 */
				const auto depth      = (desc.declarationOffset >> 25u);
				const auto realOffset = 4 * (desc.declarationOffset & 0xFFFFFFu);
				gmsFileReader->seek(realOffset);

				GMSGeomEntity::deserialize(currentGeomEntry, depth, gmsFileReader, bufFileReader);

				const bool isGroup = currentGeomEntry.isInheritedOfGeom();
				//TODO: Complete me
			}
		}
	}
}