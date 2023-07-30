#include <GameLib/TEX/TEXWriter.h>
#include <ZBinaryWriter.hpp>


namespace gamelib::tex
{
	void TEXWriter::write(const TEXHeader &header, const std::vector<TEXEntry> &entries, std::vector<uint8_t> &outBuffer)
	{
		auto writerSink = std::make_unique<ZBio::ZBinaryWriter::BufferSink>();
		auto binaryWriter = ZBio::ZBinaryWriter::BinaryWriter(std::move(writerSink));

		// Offsets & pools
		uint32_t texturesPoolOffset = 0u;
		uint32_t cubeMapsPoolOffset = 0u;

		OffsetsPool textureOffsetsPool { 0u };
		OffsetsPool cubeMapsOffsetsPool { 0u };

		// Write header (will modify it later)
		TEXHeader::serialize(header, &binaryWriter);

		// Write entry by entry
		for (size_t entryIndex = 0; entryIndex < entries.size(); ++entryIndex) // NOLINT(modernize-loop-convert)
		{
			const auto& entry = entries[entryIndex];
			TEXEntry::serialize(entry, &binaryWriter, textureOffsetsPool[entry.m_index], cubeMapsOffsetsPool[entry.m_index]);
		}

		// Next is going offsets table
		texturesPoolOffset = binaryWriter.tell();
		binaryWriter.write<uint32_t, ZBio::Endianness::LE>(textureOffsetsPool.data(), static_cast<int64_t>(textureOffsetsPool.size()));

		// And write cube map indices pool
		cubeMapsPoolOffset = binaryWriter.tell();
		binaryWriter.write<uint32_t, ZBio::Endianness::LE>(cubeMapsOffsetsPool.data(), static_cast<int64_t>(cubeMapsOffsetsPool.size()));

		// Finally, copy data to final buffer
		auto raw = binaryWriter.release().value();
		std::copy(raw.begin(), raw.end(), std::back_inserter(outBuffer));

		// And replace offsets (classic)
		auto* finalHeader = reinterpret_cast<TEXHeader*>(outBuffer.data());
		finalHeader->m_texturesPoolOffset = texturesPoolOffset;
		finalHeader->m_cubeMapsPoolOffset = cubeMapsPoolOffset;
	}
}