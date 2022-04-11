#pragma once

#include <cstdint>
#include <vector>


namespace gamelib::prp {
	class PRPHeader
	{
	public:
		PRPHeader() = default;
		PRPHeader(uint32_t totalKeys, bool isRaw, bool isSave = false, bool isTokenTablePresented = true);
		PRPHeader(const uint8_t *data, int64_t size);

		[[nodiscard]] bool isValid() const;
		[[nodiscard]] bool isRaw() const;
		[[nodiscard]] bool isSave() const;
		[[nodiscard]] bool isTokenTablePresented() const;
		[[nodiscard]] uint32_t getFlags() const;
		[[nodiscard]] uint32_t getTotalKeys() const;
		[[nodiscard]] uint32_t getZDefinesOffset() const;

		[[nodiscard]] explicit operator bool() const noexcept;

	private:
		uint8_t m_isMagicBytesValid { 0 };
		uint8_t m_isRawView { 0 };
		uint32_t m_flags { 0 };
		uint32_t m_totalKeys { 0 };
		uint32_t m_ZDefinesOffset { 0 };
	};
}