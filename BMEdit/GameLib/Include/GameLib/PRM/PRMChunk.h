#pragma once

#include <GameLib/PRM/PRMChunkRecognizedKind.h>
#include <GameLib/Span.h>
#include <memory>

namespace gamelib::prm
{
	class PRMChunk
	{
	private:
		std::uint32_t m_chunkIndex { 0u };
		std::unique_ptr<uint8_t[]> m_buffer { nullptr };
		std::size_t m_bufferSize { 0 };
		PRMChunkRecognizedKind m_recognizedKind { PRMChunkRecognizedKind::CRK_UNKNOWN_BUFFER };

	public:
		PRMChunk();
		PRMChunk(std::uint32_t chunkIndex, std::unique_ptr<uint8_t[]> &&buffer, std::size_t size, PRMChunkRecognizedKind recognizedKind);

		[[nodiscard]] std::uint32_t getIndex() const;
		[[nodiscard]] Span<uint8_t> getBuffer();
		[[nodiscard]] PRMChunkRecognizedKind getKind() const;
	};
}