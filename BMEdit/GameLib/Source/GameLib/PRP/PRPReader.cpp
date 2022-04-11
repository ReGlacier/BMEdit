#include <GameLib/PRP/PRPReader.h>
#include <ZBinaryReader.hpp>


namespace gamelib::prp
{
	constexpr std::size_t kHeaderOffset = 0x1F;

	bool PRPReader::parse(const uint8_t *prpFile, int64_t prpFileSize)
	{
		m_header = PRPHeader(prpFile, prpFileSize);
		if (!m_header) {
			return false;
		}

		if (m_header.isTokenTablePresented() && m_header.getTotalKeys()) {
			m_tokenTable =
				PRPTokenTable(&prpFile[kHeaderOffset], prpFileSize - kHeaderOffset, m_header.getTotalKeys());
		}

		{
			using namespace ZBio;

			ZBinaryReader::BinaryReader reader
				{reinterpret_cast<const char *>(&prpFile[kHeaderOffset]), (int64_t) (prpFileSize - kHeaderOffset)};
			m_objectsCount = reader.read<uint32_t, Endianness::LE>();
		}

		// Read ZDefines
		PRPZDefines::ReadResult zDefinesReadResult;
		{
			m_ZDefines = PRPZDefines();
			// 1F - header, 4 - counter
			const auto zDefinesOffset = 0x1F + 0x4 + m_header.getZDefinesOffset();
			const uint8_t *data = &prpFile[zDefinesOffset];
			const auto size = (int64_t) (prpFileSize - zDefinesOffset);
			const PRPTokenTable *tokenTable = m_header.isTokenTablePresented() ? &m_tokenTable : nullptr;

			m_ZDefines.read(data, size, tokenTable, zDefinesReadResult);
			if (!zDefinesReadResult) {
				return false;
			}

			zDefinesReadResult.lastOffset += zDefinesOffset;
		}

		// Read Instructions
		{
			m_byteCode = PRPByteCode();
			if (!m_byteCode.parse(
				&prpFile[zDefinesReadResult.lastOffset],
				prpFileSize - zDefinesReadResult.lastOffset,
				&m_header,
				&m_tokenTable)) {
				return false;
			}
		}

		return true;
	}

	const PRPHeader &PRPReader::getHeader() const
	{
		return m_header;
	}

	const PRPTokenTable &PRPReader::getTokenTable() const
	{
		return m_tokenTable;
	}

	uint32_t PRPReader::getObjectsCount() const
	{
		return m_objectsCount;
	}

	const PRPZDefines &PRPReader::getDefinitions() const
	{
		return m_ZDefines;
	}

	const PRPByteCode &PRPReader::getByteCode() const
	{
		return m_byteCode;
	}
}