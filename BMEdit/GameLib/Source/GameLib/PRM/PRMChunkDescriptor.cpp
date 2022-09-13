#include <GameLib/PRM/PRMChunkDescriptor.h>
#include <ZBinaryReader.hpp>


namespace gamelib::prm
{
	void PRMChunkDescriptor::deserialize(PRMChunkDescriptor &descriptor, ZBio::ZBinaryReader::BinaryReader *binaryReader)
	{
		auto &[offset, size, kind, unkC] = descriptor;

		offset = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		size   = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		kind   = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		unkC   = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
	}
}