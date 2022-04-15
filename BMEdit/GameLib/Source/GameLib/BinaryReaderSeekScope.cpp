#include <GameLib/BinaryReaderSeekScope.h>
#include <ZBinaryReader.hpp>


namespace gamelib
{
	BinaryReaderSeekScope::BinaryReaderSeekScope(ZBio::ZBinaryReader::BinaryReader *binaryReader)
		: m_binaryReader(binaryReader)
	{
		if (m_binaryReader)
		{
			m_offset = binaryReader->tell();
		}
	}

	BinaryReaderSeekScope::~BinaryReaderSeekScope()
	{
		if (m_binaryReader)
		{
			m_binaryReader->seek(m_offset);
		}
	}
}