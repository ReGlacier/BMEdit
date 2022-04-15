#pragma once

#include <cstdint>
#include <memory>

#include <GameLib/GMS/GMSHeader.h>


namespace gamelib::gms
{
	class GMSReader
	{
	public:
		GMSReader();

		bool parse(const uint8_t *gmsBuffer, int64_t gmsBufferSize, const uint8_t *bufBuffer, int64_t bufBufferSize);

		[[nodiscard]] const GMSHeader &getHeader() const;

	private:
		[[nodiscard]] static std::unique_ptr<uint8_t[]> decompressGmsBuffer(const uint8_t *rawBuffer, uint32_t rawBufferSize, uint32_t uncompressedSize);
		[[nodiscard]] bool prepareGmsFileBody(const uint8_t *gmsFile, int64_t gmsFileSize, const uint8_t *bufBuffer, int64_t bufBufferSize);

	private:
		GMSHeader m_header;
	};
}