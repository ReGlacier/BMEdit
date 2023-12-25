#include <GameLib/TEX/TEXHeader.h>
#include <ZBinaryReader.hpp>
#include <ZBinaryWriter.hpp>


namespace gamelib::tex
{
	TEXHeader::TEXHeader(uint32_t table1Offset, uint32_t table2Offset, uint32_t unk1, uint32_t unk2)
		: m_texturesPoolOffset(table1Offset)
		, m_cubeMapsPoolOffset(table2Offset)
		, m_unk1(unk1)
		, m_unk2(unk2)
	{
	}

	void TEXHeader::deserialize(TEXHeader &header, ZBio::ZBinaryReader::BinaryReader *binaryReader)
	{
		header.m_texturesPoolOffset = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		header.m_cubeMapsPoolOffset = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		header.m_unk1 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		header.m_unk2 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
	}

	void TEXHeader::serialize(const TEXHeader &header, ZBio::ZBinaryWriter::BinaryWriter *writerStream)
	{
		writerStream->write<uint32_t, ZBio::Endianness::LE>(header.m_texturesPoolOffset);
		writerStream->write<uint32_t, ZBio::Endianness::LE>(header.m_cubeMapsPoolOffset);
		writerStream->write<uint32_t, ZBio::Endianness::LE>(header.m_unk1);
		writerStream->write<uint32_t, ZBio::Endianness::LE>(header.m_unk2);
	}
}