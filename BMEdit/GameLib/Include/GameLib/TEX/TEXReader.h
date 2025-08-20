#pragma once

#include <GameLib/TEX/TEXHeader.h>
#include <GameLib/TEX/TEXEntry.h>
#include <GameLib/TEX/TEXTypes.h>

#include <cstdint>
#include <vector>
#include <array>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::tex
{
	class TEXReader
	{
	public:
		TEXReader() = default;

		bool parse(const uint8_t *texFileBuffer, int64_t texFileSize);

	public:
		TEXHeader m_header;
		std::vector<TEXEntry> m_entries;
		OffsetsPool m_texturesPool { 0u };
		OffsetsPool m_cubeMapsPool { 0u };
		uint32_t m_countOfEmptyOffsets { 0u };
	};
}