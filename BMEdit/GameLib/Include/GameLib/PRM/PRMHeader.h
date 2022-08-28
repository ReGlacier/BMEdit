#pragma once

#include <cstdint>



namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::prm
{
	struct PRMHeader
	{
		uint32_t chunkOffset { 0 }; // Header + 0x0: Offset of main chunk
		uint32_t countOfPrimitives { 0 }; // Header + 0x4: Total geoms count on scene (max about 0x21000 things)
		uint32_t unk8 {0}; // Duplicate of chunkOffset, maybe second chunk? Or ... LODs?
		uint32_t zeroed {0}; // always zero (maybe used as 'checkpoint')

		static void deserialize(PRMHeader &header, ZBio::ZBinaryReader::BinaryReader *binaryReader);
	};
}