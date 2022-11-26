#include <GameLib/PRM/PRMIndexChunkHeader.h>
#include <ZBinaryReader.hpp>


namespace gamelib::prm
{
	void PRMIndexChunkHeader::deserialize(gamelib::prm::PRMIndexChunkHeader &header, ZBio::ZBinaryReader::BinaryReader *binaryReader)
	{
		header.unk0 = binaryReader->read<std::uint16_t, ZBio::Endianness::LE>();
		header.indicesCount = binaryReader->read<std::uint16_t, ZBio::Endianness::LE>();
	}
}