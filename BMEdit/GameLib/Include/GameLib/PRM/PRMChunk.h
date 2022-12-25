#pragma once

#include <GameLib/PRM/PRMDescriptionChunkBaseHeader.h>
#include <GameLib/PRM/PRMChunkRecognizedKind.h>
#include <GameLib/PRM/PRMVertexBufferHeader.h>
#include <GameLib/PRM/PRMIndexChunkHeader.h>
#include <GameLib/PRM/PRMVertexFormatDetails.h>
#include <GameLib/Span.h>
#include <variant>
#include <memory>


namespace gamelib::prm
{
	class PRMChunk final
	{
	private:
		struct NullData {};

		struct MetaData
		{
			PRMDescriptionChunkBaseHeader header;
		};

		struct BoneData
		{
			int unused { 0 };
		};

		struct VertexData
		{
			PRMVertexBufferHeader header {};
			PRMVertexFormatDetails details {};
		};

		struct IndexData
		{
			PRMIndexChunkHeader header;
		};

		std::uint32_t m_chunkIndex { 0u };
		std::unique_ptr<uint8_t[]> m_buffer { nullptr };
		std::size_t m_bufferSize { 0 };
		PRMChunkRecognizedKind m_recognizedKind { PRMChunkRecognizedKind::CRK_UNKNOWN_BUFFER };
		std::variant<NullData, MetaData, BoneData, VertexData, IndexData> m_data;

	public:
		PRMChunk();
		PRMChunk(std::uint32_t chunkIndex, int totalChunksNr, std::unique_ptr<uint8_t[]> &&buffer, std::size_t size);

		[[nodiscard]] std::uint32_t getIndex() const;
		[[nodiscard]] Span<uint8_t> getBuffer();
		[[nodiscard]] std::size_t getBufferSize() const;
		[[nodiscard]] PRMChunkRecognizedKind getKind() const;

		// Meta data
		[[nodiscard]] const PRMDescriptionChunkBaseHeader* getDescriptionBufferHeader() const;
		[[nodiscard]] PRMDescriptionChunkBaseHeader* getDescriptionBufferHeader();

		// Index data
		[[nodiscard]] const PRMIndexChunkHeader* getIndexBufferHeader() const;
		[[nodiscard]] PRMIndexChunkHeader* getIndexBufferHeader();

		// Vertex data
		[[nodiscard]] const PRMVertexBufferHeader* getVertexBufferHeader() const;
		[[nodiscard]] PRMVertexBufferHeader* getVertexBufferHeader();
		[[nodiscard]] const PRMVertexFormatDetails* getVertexFormatDetails() const;
		[[nodiscard]] PRMVertexFormatDetails* getVertexFormatDetails();

		// Bone data
	private:
		void recognizeChunkKindAndSaveData(Span<uint8_t> chunk, int totalChunksNr);
	};
}