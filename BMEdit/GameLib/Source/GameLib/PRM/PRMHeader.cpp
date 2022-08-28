#include <GameLib/PRM/PRMHeader.h>
#include <ZBinaryReader.hpp>


namespace gamelib::prm
{
	void PRMHeader::deserialize(gamelib::prm::PRMHeader &header, ZBio::ZBinaryReader::BinaryReader *binaryReader)
	{
		header.chunkOffset = binaryReader->read<uint32_t>();
		header.countOfPrimitives = binaryReader->read<uint32_t>();
		header.unk8 = binaryReader->read<uint32_t>();
		header.zeroed = binaryReader->read<uint32_t>();
	}
}