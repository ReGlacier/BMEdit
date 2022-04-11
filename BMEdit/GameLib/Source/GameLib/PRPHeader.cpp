#include <GameLib/PRPHeader.h>
#include <ZBinaryReader.hpp>
#include <array>


namespace gamelib::prp
{
	PRPHeader::PRPHeader(uint32_t totalKeys, bool isRaw, bool isSave, bool isTokenTablePresented)
		: m_isMagicBytesValid(true)
		, m_isRawView(isRaw)
		, m_totalKeys(totalKeys)
	{
		if (isSave) {
			m_flags |= 1 << 1;
		}

		m_flags = 0xD; // set to normal 0xD (1101)
		if (isSave) { m_flags |= 1 << 1; }
		if (!totalKeys || !isTokenTablePresented) { m_flags &= ~(1 << 3); }
	}

	PRPHeader::PRPHeader(const uint8_t *data, int64_t size)
	{
		using namespace ZBio;

		ZBinaryReader::BinaryReader reader { reinterpret_cast<const char*>(data), size };

		// Read magic bytes
		std::array<char, 14> magicBytes {};
		constexpr std::array<char, 14> kRequiredMagicBytes { 'I', 'O', 'P', 'a', 'c', 'k', 'e', 'd', ' ', 'v', '0', '.', '1', 0x0 };
		reader.read<char, Endianness::LE>(&magicBytes[0], 14);
		m_isMagicBytesValid = magicBytes == kRequiredMagicBytes;

		if (!m_isMagicBytesValid) {
			return;
		}

		m_isRawView = reader.read<bool, Endianness::LE>();
		m_flags = reader.read<uint32_t, Endianness::LE>();
		reader.seek(0x17); // Jump to 0x17
		m_totalKeys = reader.read<uint32_t, Endianness::LE>();
		m_ZDefinesOffset = reader.read<uint32_t, Endianness::LE>();

		assert(reader.tell() == 0x1F); // Check that we are in right position
	}

	bool PRPHeader::isValid() const
	{
		return m_isMagicBytesValid;
	}

	bool PRPHeader::isRaw() const
	{
		return m_isRawView;
	}

	bool PRPHeader::isSave() const
	{
		return m_flags & (1 << 1);
	}

	bool PRPHeader::isTokenTablePresented() const
	{
		return m_flags & (1 << 3);
	}

	uint32_t PRPHeader::getFlags() const
	{
		return m_flags;
	}

	uint32_t PRPHeader::getTotalKeys() const
	{
		return m_totalKeys;
	}

	uint32_t PRPHeader::getZDefinesOffset() const
	{
		return m_ZDefinesOffset;
	}

	PRPHeader::operator bool() const noexcept
	{
		return m_isMagicBytesValid;
	}
}