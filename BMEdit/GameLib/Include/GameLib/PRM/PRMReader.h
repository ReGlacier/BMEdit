#pragma once

#include <GameLib/PRM/PRMHeader.h>
#include <GameLib/PRM/PRMChunk.h>
#include <GameLib/PRM/PRMChunkDescriptor.h>
#include <GameLib/Span.h>
#include <cstdint>
#include <vector>


namespace gamelib::prm
{
	class PRMReader
	{
	public:
		PRMReader() = delete;
		PRMReader(PRMHeader &header, std::vector<PRMChunkDescriptor> &chunkDescriptors, std::vector<PRMChunk> &chunks);

		bool read(Span<uint8_t> buffer);

		[[nodiscard]] const PRMHeader &getHeader() const;
		[[nodiscard]] const std::vector<PRMChunkDescriptor> &getChunkDescriptors() const;
		[[nodiscard]] PRMChunk* getChunkAt(size_t chunkIndex);
		[[nodiscard]] const PRMChunk* getChunkAt(size_t chunkIndex) const;

	private:
		PRMHeader& m_header;
		std::vector<PRMChunk>& m_chunks;
		std::vector<PRMChunkDescriptor>& m_chunkDescriptors;
	};
}