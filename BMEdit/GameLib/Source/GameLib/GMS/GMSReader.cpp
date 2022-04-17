#include <GameLib/GMS/GMSReader.h>
#include <ZBinaryReader.hpp>

extern "C" {
#include <zlib.h>
}


namespace gamelib::gms
{
	GMSReader::GMSReader() = default;

	bool GMSReader::parse(const GMSHeader *header, const uint8_t *gmsBuffer, int64_t gmsBufferSize, const uint8_t *bufBuffer, int64_t bufBufferSize)
	{
		m_header = header;

		// Read RAW header (first 9 bytes)
		ZBio::ZBinaryReader::BinaryReader reader(reinterpret_cast<const char *>(gmsBuffer), gmsBufferSize);
		const auto uncompressedSize = reader.read<uint32_t, ZBio::Endianness::LE>();
		const auto bufferSize = reader.read<uint32_t, ZBio::Endianness::LE>();
		const auto canAvoidUncompressOperation = reader.read<bool, ZBio::Endianness::LE>();

		std::unique_ptr<uint8_t[]> decompressedGmsBody = nullptr;
		const uint8_t *gmsFileBody = (gmsBuffer + 0x9);
		int64_t gmsFileBodySize = gmsBufferSize - 0x9;

		if (!canAvoidUncompressOperation)
		{
			// Need to decompress GMS body
			decompressedGmsBody = decompressGmsBuffer(gmsBuffer + 0x9, bufferSize, uncompressedSize);
			if (!decompressedGmsBody)
			{
				return false;
			}

			gmsFileBody = decompressedGmsBody.get();
			gmsFileBodySize = uncompressedSize;
		}

		// Header reversed, body decompressed, ready to prepare contents
		return prepareGmsFileBody(gmsFileBody, gmsFileBodySize, bufBuffer, bufBufferSize);
	}

	std::unique_ptr<uint8_t []> GMSReader::decompressGmsBuffer(const uint8_t *rawBuffer,
	                                                           uint32_t rawBufferSize,
	                                                           uint32_t uncompressedSize)
	{
		auto outBuffer = std::make_unique<uint8_t[]>(uncompressedSize);
		if (!outBuffer)
		{
			return nullptr;
		}

		z_stream stream;
		stream.avail_in = rawBufferSize;
		stream.next_in = const_cast<uint8_t *>(rawBuffer);
		stream.next_out = outBuffer.get();
		stream.avail_out = (uncompressedSize + 0x1F) & 0xFFFFFFF0;
		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;

		if (inflateInit2(&stream, -15) != Z_OK)
		{
			return nullptr;
		}

		int result = inflate(&stream, Z_FINISH);
		inflateEnd(&stream);

		if (result == Z_NEED_DICT || result == Z_DATA_ERROR || result == Z_MEM_ERROR)
		{
			//NOTE: Raise exception?
			return nullptr;
		}

		return outBuffer;
	}

	bool GMSReader::prepareGmsFileBody(const uint8_t *gmsFile, int64_t gmsFileSize, const uint8_t *bufBuffer, int64_t bufBufferSize)
	{
		if (!m_header)
		{
			//NOTE: Assert here!
			return false;
		}

		ZBio::ZBinaryReader::BinaryReader gmsBinaryReader { reinterpret_cast<const char *>(gmsFile), gmsFileSize };
		ZBio::ZBinaryReader::BinaryReader bufBinaryReader { reinterpret_cast<const char *>(bufBuffer), bufBufferSize };

		// Now we have a pure GMS body and we are ready to read all data
		GMSHeader::deserialize(*const_cast<GMSHeader *>(m_header), &gmsBinaryReader, &bufBinaryReader);
		return true;
	}
}