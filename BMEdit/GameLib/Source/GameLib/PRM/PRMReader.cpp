#include <GameLib/PRM/PRMChunkDescriptor.h>
#include <GameLib/PRM/PRMChunk.h>
#include <GameLib/PRM/PRMReader.h>
#include <ZBinaryReader.hpp>


namespace gamelib::prm
{
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
			assert(false && "Zeroed field must be zeroed!");
			return false;
		}

		if (m_header.countOfPrimitives >= 40960) // There are 40960 geoms max. Need to check it later
		{
			assert(false && "Possibly invalid PRM file. Game supports max 40959 unique primitives per level");
			return false;
		}

		m_chunks.reserve(m_header.countOfPrimitives);
		m_chunkDescriptors.reserve(m_header.countOfPrimitives);

		for (int chunkIndex = 0; chunkIndex < m_header.countOfPrimitives; ++chunkIndex)
		{
			if (m_header.chunkOffset + (chunkIndex * PRMChunkDescriptor::kDescriptorSize) >= buffer.size())
			{
				assert(false && "Invalid chunk offset! Bad PRM file");
				return false;
			}

			// Read chunk descriptor
			binaryReader.seek(m_header.chunkOffset + (chunkIndex * PRMChunkDescriptor::kDescriptorSize));
			auto &descriptor = m_chunkDescriptors.emplace_back();
			PRMChunkDescriptor::deserialize(descriptor, &binaryReader);

			// Read chunk
			m_chunks.emplace_back(buffer.slice(descriptor.declarationOffset, descriptor.declarationSize).new_buffer());
		}

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

	PRMRawChunk PRMReader::getChunkAt(size_t chunkIndex)
	{
		return chunkIndex >= m_chunks.size() ? nullptr : m_chunks.at(chunkIndex).get();
	}

	const PRMRawChunk PRMReader::getChunkAt(size_t chunkIndex) const
	{
		return chunkIndex >= m_chunks.size() ? nullptr : m_chunks.at(chunkIndex).get();
	}
}