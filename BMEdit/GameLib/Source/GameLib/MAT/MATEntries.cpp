#include <GameLib/MAT/MATEntries.h>
#include <ZBinaryReader.hpp>


namespace gamelib::mat
{
	void MATHeader::deserialize(MATHeader& header, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		header.classListOffset     = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		header.instancesListOffset = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		header.zeroed              = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		header.unknownTableOffset  = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
	}

	void MATPropertyEntry::deserialize(MATPropertyEntry& entry, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		entry.kind = static_cast<MATPropertyKind>(binaryReader->read<uint32_t, ZBio::Endianness::LE>());
		entry.reference = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		entry.containerCapacity = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		entry.valueType = static_cast<MATValueType>(binaryReader->read<uint32_t, ZBio::Endianness::LE>());
	}

	void MATClassDescription::deserialize(MATClassDescription& classDescription, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		{
			const auto offset = binaryReader->read<uint32_t, ZBio::Endianness::LE>();

			if (offset >= 0x10)
			{
				const auto oldPos = binaryReader->tell();

				// Read class name from valid offset
				binaryReader->seek(offset);
				classDescription.parentClass = binaryReader->readCString();

				// Move back
				binaryReader->seek(oldPos);
			}
		}

		classDescription.unk4 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		classDescription.unk8 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		classDescription.unkC = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		classDescription.unk10 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		classDescription.unk14 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		classDescription.unk18 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		classDescription.classDeclarationOffset = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		classDescription.unk20 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		classDescription.unk24 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		classDescription.unk28 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		classDescription.unk2C = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
	}

	void MATInstanceDescription::deserialize(MATInstanceDescription& instanceDescription, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		{
			const auto offset = binaryReader->read<uint32_t, ZBio::Endianness::LE>();

			if (offset >= 0x10)
			{
				const auto oldPos = binaryReader->tell();

				// Read class name from valid offset
				binaryReader->seek(offset);
				instanceDescription.instanceParentClassName = binaryReader->readCString();

				// Move back
				binaryReader->seek(oldPos);
			}
		}

		instanceDescription.unk4 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		instanceDescription.unk8 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		instanceDescription.unkC = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		instanceDescription.unk10 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		instanceDescription.unk14 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		instanceDescription.unk18 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		instanceDescription.instanceDeclarationOffset = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		instanceDescription.unk20 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		instanceDescription.unk24 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		instanceDescription.unk28 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		instanceDescription.unk2C = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
	}
}
