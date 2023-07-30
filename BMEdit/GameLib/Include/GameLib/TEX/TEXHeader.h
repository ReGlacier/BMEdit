#pragma once

#include <cstdint>


namespace ZBio::ZBinaryWriter
{
	class BinaryWriter;
}

namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::tex
{
	struct TEXHeader
	{
		TEXHeader() = default;
		TEXHeader(uint32_t table1Offset, uint32_t table2Offset, uint32_t unk1, uint32_t unk2);

		[[nodiscard]] explicit operator bool() const noexcept;

		static void deserialize(TEXHeader &header, ZBio::ZBinaryReader::BinaryReader *binaryReader);
		static void serialize(const TEXHeader &header, ZBio::ZBinaryWriter::BinaryWriter *writerStream);

	public:
		uint32_t m_texturesPoolOffset { 0u };
		uint32_t m_cubeMapsPoolOffset { 0u };
		uint32_t m_unk1 { 0u };
		uint32_t m_unk2 { 0u };
	};
}