#include <GameLib/PRM/PRMDescriptionChunkBaseHeader.h>
#include <ZBinaryReader.hpp>


using namespace gamelib::prm;

void PRMDescriptionChunkBaseHeader::deserialize(gamelib::prm::PRMDescriptionChunkBaseHeader &header, ZBio::ZBinaryReader::BinaryReader *binaryReader)
{
	header.boneDeclOffset = binaryReader->read<std::uint8_t, ZBio::Endianness::LE>();
	header.primPackType = binaryReader->read<std::uint8_t, ZBio::Endianness::LE>();
	header.kind = binaryReader->read<std::uint16_t, ZBio::Endianness::LE>();
	header.textureId = binaryReader->read<std::uint16_t, ZBio::Endianness::LE>();
	header.unk6 = binaryReader->read<std::uint16_t, ZBio::Endianness::LE>();
	header.nextVariation = binaryReader->read<std::uint32_t, ZBio::Endianness::LE>();
	header.unkC = binaryReader->read<std::uint8_t, ZBio::Endianness::LE>();
	header.unkD = binaryReader->read<std::uint8_t, ZBio::Endianness::LE>();
	header.unkE = binaryReader->read<std::uint8_t, ZBio::Endianness::LE>();
	header.currentVariation = binaryReader->read<std::uint8_t, ZBio::Endianness::LE>();
	header.ptrParts = binaryReader->read<std::uint16_t, ZBio::Endianness::LE>();
	header.materialIdx = binaryReader->read<std::uint16_t, ZBio::Endianness::LE>();
	header.totalVariations = binaryReader->read<std::uint32_t, ZBio::Endianness::LE>();
	header.ptrObjects = binaryReader->read<std::uint16_t, ZBio::Endianness::LE>();
	header.ptrObjects_HI = binaryReader->read<std::uint16_t, ZBio::Endianness::LE>();
	header.unk3 = binaryReader->read<std::uint32_t, ZBio::Endianness::LE>();
	header.boundingBox.min.x = binaryReader->read<float, ZBio::Endianness::LE>();
	header.boundingBox.min.y = binaryReader->read<float, ZBio::Endianness::LE>();
	header.boundingBox.min.z = binaryReader->read<float, ZBio::Endianness::LE>();
	header.boundingBox.max.x = binaryReader->read<float, ZBio::Endianness::LE>();
	header.boundingBox.max.y = binaryReader->read<float, ZBio::Endianness::LE>();
	header.boundingBox.max.z = binaryReader->read<float, ZBio::Endianness::LE>();
}