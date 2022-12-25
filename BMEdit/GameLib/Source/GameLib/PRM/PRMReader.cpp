#include <GameLib/PRM/PRMChunkDescriptor.h>
#include <GameLib/BinaryReaderSeekScope.h>
#include <GameLib/PRM/PRMBadChunkException.h>
#include <GameLib/PRM/PRMBadFile.h>
#include <GameLib/PRM/PRMReader.h>
#include <GameLib/PRM/PRMChunk.h>
#include <GameLib/PRM/PRMDescriptionChunkBaseHeader.h>

#include <ZBinaryReader.hpp>


namespace gamelib::prm
{
	constexpr std::size_t kMaxChunksPerFile = 40960; // There are 40960 geoms max

	PRMReader::PRMReader(gamelib::prm::PRMHeader &header, std::vector<PRMChunkDescriptor> &chunkDescriptors, std::vector<PRMChunk> &chunks)
		: m_header(header)
		, m_chunkDescriptors(chunkDescriptors)
		, m_chunks(chunks)
	{
	}

	bool PRMReader::read(Span<uint8_t> buffer)
	{
		if (!buffer)
		{
			return false;
		}

		ZBio::ZBinaryReader::BinaryReader binaryReader(reinterpret_cast<const char*>(buffer.data()), buffer.size());

		// Read header
		PRMHeader::deserialize(m_header, &binaryReader);
		if (m_header.zeroed != 0x0)
		{
			throw PRMBadFile("Zeroed field must be zeroed!");
		}

		if (m_header.countOfPrimitives >= kMaxChunksPerFile)
		{
			throw PRMBadFile("Possibly invalid PRM file. Game supports max 40959 unique primitives per level");
		}

		m_chunks.reserve(m_header.countOfPrimitives);
		m_chunkDescriptors.reserve(m_header.countOfPrimitives);

		for (std::uint32_t chunkIndex = 0u; chunkIndex < m_header.countOfPrimitives; ++chunkIndex)
		{
			if (m_header.chunkOffset + (chunkIndex * PRMChunkDescriptor::kDescriptorSize) >= buffer.size())
			{
				throw PRMBadChunkException(chunkIndex);
			}

			// Read chunk descriptor
			binaryReader.seek(m_header.chunkOffset + (chunkIndex * PRMChunkDescriptor::kDescriptorSize));
			auto &descriptor = m_chunkDescriptors.emplace_back();
			PRMChunkDescriptor::deserialize(descriptor, &binaryReader);

			// Read chunk
			auto chunkBufferSize = descriptor.declarationSize;
			auto chunkBuffer = std::make_unique<uint8_t[]>(chunkBufferSize);

			{
				BinaryReaderSeekScope scope { &binaryReader };

				binaryReader.seek(descriptor.declarationOffset);
				binaryReader.read<std::uint8_t>(&chunkBuffer[0], static_cast<std::int64_t>(chunkBufferSize));
			}

			m_chunks.emplace_back(chunkIndex, m_header.countOfPrimitives, std::move(chunkBuffer), chunkBufferSize);
		}

#if 0 // This part of code commented because we allow to have unrecognized segments. Just ignore 'em all.
 		int unrecognizedChunks = 0;
		for (const auto& chk: m_chunks)
		{
			if (chk.getKind() == PRMChunkRecognizedKind::CRK_UNKNOWN_BUFFER)
			{
				unrecognizedChunks++;
			}
		}

		if (unrecognizedChunks > 0)
		{
			throw PRMBadFile("Found at least 1 unrecognized chunk. Need to check level by devs");
		}
#endif
		return true;
	}

	const PRMHeader &PRMReader::getHeader() const
	{
		return m_header;
	}

	const std::vector<PRMChunkDescriptor> &PRMReader::getChunkDescriptors() const
	{
		return m_chunkDescriptors;
	}

	PRMChunk* PRMReader::getChunkAt(size_t chunkIndex)
	{
		return chunkIndex >= m_chunks.size() ? nullptr : &m_chunks[chunkIndex];
	}

	const PRMChunk* PRMReader::getChunkAt(size_t chunkIndex) const
	{
		return chunkIndex >= m_chunks.size() ? nullptr : &m_chunks[chunkIndex];
	}
}