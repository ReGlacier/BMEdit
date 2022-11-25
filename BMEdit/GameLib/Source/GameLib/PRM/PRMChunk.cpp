#include <GameLib/PRM/PRMChunk.h>


namespace gamelib::prm
{
	PRMChunk::PRMChunk() = default;

	PRMChunk::PRMChunk(std::uint32_t chunkIndex, std::unique_ptr<uint8_t[]> &&buffer, std::size_t size, gamelib::prm::PRMChunkRecognizedKind recognizedKind)
	    : m_chunkIndex(chunkIndex)
	    , m_buffer(std::move(buffer))
		, m_bufferSize(size)
		, m_recognizedKind(recognizedKind)
	{
	}

	std::uint32_t PRMChunk::getIndex() const
	{
		return m_chunkIndex;
	}

	Span<uint8_t> PRMChunk::getBuffer()
	{
		return { m_buffer.get(), static_cast<int64_t>(m_bufferSize) };
	}

	PRMChunkRecognizedKind PRMChunk::getKind() const
	{
		return m_recognizedKind;
	}
}