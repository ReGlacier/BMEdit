#include <GameLib/OCT/OCTEntries.h>
#include <ZBinaryReader.hpp>
#include <glm/gtc/type_ptr.hpp>



namespace gamelib::oct
{
	void OCTHeader::deserialize(OCTHeader& header, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		header.objectsOffset = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		binaryReader->read<float, ZBio::Endianness::LE>(glm::value_ptr(header.vWorldOrigin), 3);
		header.fWorldScale = binaryReader->read<float, ZBio::Endianness::LE>();
	}

	void OCTNode::deserialize(OCTNode& node, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		node.childCount = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		node.childIndex = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		node.objectIndex = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
	}

	void OCTObject::deserialize(OCTObject& object, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		object.gameObjectREF = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		binaryReader->read<int16_t, ZBio::Endianness::LE>(glm::value_ptr(object.vMin), 3);
		binaryReader->read<int16_t, ZBio::Endianness::LE>(glm::value_ptr(object.vMax), 3);
	}

	void OCTUnknownBlock::deserialize(gamelib::oct::OCTUnknownBlock& block, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		block.unk0  = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		block.unk4  = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unk8  = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unkC  = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unk10 = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unk14 = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unk18 = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unk1C = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unk20 = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unk24 = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unk28 = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unk2C = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unk30 = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unk34 = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unk38 = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unk3C = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unk40 = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unk44 = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unk48 = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unk4C = binaryReader->read<float, ZBio::Endianness::LE>();
		block.unk50 = binaryReader->read<float, ZBio::Endianness::LE>();
	}
}