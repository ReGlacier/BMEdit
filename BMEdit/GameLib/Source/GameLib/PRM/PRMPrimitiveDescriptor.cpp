#include <GameLib/PRM/PRMPrimitiveDescriptor.h>
#include <ZBinaryReader.hpp>


namespace gamelib::prm
{
	void PRMPrimitiveDescriptor::deserialize(PRMPrimitiveDescriptor &descriptor, ZBio::ZBinaryReader::BinaryReader *binaryReader)
	{
		auto &[offset, size, shouldBeLoaded, unkC] = descriptor;

		offset = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		size = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		shouldBeLoaded = static_cast<bool>(binaryReader->read<uint32_t, ZBio::Endianness::LE>());
		unkC = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
	}
}