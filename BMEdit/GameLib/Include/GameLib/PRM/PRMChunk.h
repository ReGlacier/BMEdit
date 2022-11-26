#pragma once

#include <GameLib/PRM/PRMDescriptionChunkBaseHeader.h>
#include <GameLib/PRM/PRMChunkRecognizedKind.h>
#include <GameLib/PRM/PRMVertexBufferHeader.h>
#include <GameLib/PRM/PRMIndexChunkHeader.h>
#include <GameLib/Span.h>
#include <variant>
#include <memory>


namespace gamelib::prm
{
	class PRMChunk
	{
	private:
		struct NullData {};

		std::uint32_t m_chunkIndex { 0u };
		std::unique_ptr<uint8_t[]> m_buffer { nullptr };
		std::size_t m_bufferSize { 0 };
		PRMChunkRecognizedKind m_recognizedKind { PRMChunkRecognizedKind::CRK_UNKNOWN_BUFFER };
		std::variant<NullData, PRMDescriptionChunkBaseHeader, PRMIndexChunkHeader, PRMVertexBufferHeader> m_data;

	public:
		PRMChunk();
		PRMChunk(std::uint32_t chunkIndex, int totalChunksNr, std::unique_ptr<uint8_t[]> &&buffer, std::size_t size);

		[[nodiscard]] std::uint32_t getIndex() const;
		[[nodiscard]] Span<uint8_t> getBuffer();
		[[nodiscard]] PRMChunkRecognizedKind getKind() const;

		[[nodiscard]] const PRMDescriptionChunkBaseHeader* getDescriptionBufferHeader() const;
		[[nodiscard]] PRMDescriptionChunkBaseHeader* getDescriptionBufferHeader();

		[[nodiscard]] const PRMIndexChunkHeader* getIndexBufferHeader() const;
		[[nodiscard]] PRMIndexChunkHeader* getIndexBufferHeader();

		[[nodiscard]] const PRMVertexBufferHeader* getVertexBufferHeader() const;
		[[nodiscard]] PRMVertexBufferHeader* getVertexBufferHeader();

	private:
		void recognizeChunkKindAndSaveData(Span<uint8_t> chunk, int totalChunksNr);
	};
}