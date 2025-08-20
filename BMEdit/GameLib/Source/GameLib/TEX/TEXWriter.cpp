#include <GameLib/TEX/TEXWriter.h>
#include <ZBinaryWriter.hpp>


namespace gamelib::tex
{
	void TEXWriter::write(const TEXHeader &header, const std::vector<TEXEntry> &entries, std::vector<uint8_t> &outBuffer)
	{
		auto writerSink = std::make_unique<ZBio::ZBinaryWriter::BufferSink>();
		auto binaryWriter = ZBio::ZBinaryWriter::BinaryWriter(std::move(writerSink));

		TEXHeader localHeader = header;
		OffsetsPool textureOffsetsPool { 0u };
		OffsetsPool cubeMapsOffsetsPool { 0u };

		// Allocate space for header (will be serialized later)
		binaryWriter.seek(sizeof(TEXHeader));

		// Write entry by entry
		for (size_t entryIndex = 0; entryIndex < entries.size(); ++entryIndex) // NOLINT(modernize-loop-convert)
		{
			const auto& entry = entries[entryIndex];
			TEXEntry::serialize(entry, &binaryWriter, textureOffsetsPool[entry.m_index], cubeMapsOffsetsPool[entry.m_index]);
		}

		// Next is going offsets table
		localHeader.m_texturesPoolOffset = binaryWriter.tell();
		binaryWriter.write<uint32_t, ZBio::Endianness::LE>(textureOffsetsPool.data(), static_cast<int64_t>(textureOffsetsPool.size()));

		// And write cube map indices pool
		localHeader.m_cubeMapsPoolOffset = binaryWriter.tell();
		binaryWriter.write<uint32_t, ZBio::Endianness::LE>(cubeMapsOffsetsPool.data(), static_cast<int64_t>(cubeMapsOffsetsPool.size()));

		// Serialize header
		binaryWriter.seek(0);
		TEXHeader::serialize(localHeader, &binaryWriter);

		// Finally, copy data to final buffer
		auto raw = binaryWriter.release().value();
		std::copy(raw.begin(), raw.end(), std::back_inserter(outBuffer));
	}
}