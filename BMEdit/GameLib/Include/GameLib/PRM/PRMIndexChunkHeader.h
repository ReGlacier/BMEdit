#pragma once

#include <cstdint>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::prm
{
	struct PRMIndexChunkHeader
	{
		std::uint16_t unk0;
		std::uint16_t indicesCount;

		static void deserialize(PRMIndexChunkHeader& header, ZBio::ZBinaryReader::BinaryReader *binaryReader);
	};
}