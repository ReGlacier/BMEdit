#include <GameLib/PRM/PRMChunk.h>
#include <ZBinaryReader.hpp>


namespace gamelib::prm
{
	PRMChunk::PRMChunk() = default;

	PRMChunk::PRMChunk(std::uint32_t chunkIndex, int totalChunksNr, std::unique_ptr<uint8_t[]> &&buffer, std::size_t size)
	    : m_chunkIndex(chunkIndex)
	    , m_buffer(std::move(buffer))
		, m_bufferSize(size)
	{
		// Recognize type & save data
		if (chunkIndex == 0u)
		{
			m_recognizedKind = PRMChunkRecognizedKind::CRK_ZERO_CHUNK;
		}
		else
		{
			recognizeChunkKindAndSaveData(getBuffer(), totalChunksNr);
		}
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

	const PRMDescriptionChunkBaseHeader* PRMChunk::getDescriptionBufferHeader() const
	{
		return std::get_if<PRMDescriptionChunkBaseHeader>(&m_data);
	}

	PRMDescriptionChunkBaseHeader* PRMChunk::getDescriptionBufferHeader()
	{
		return std::get_if<PRMDescriptionChunkBaseHeader>(&m_data);
	}

	const PRMIndexChunkHeader* PRMChunk::getIndexBufferHeader() const
	{
		return std::get_if<PRMIndexChunkHeader>(&m_data);
	}

	PRMIndexChunkHeader* PRMChunk::getIndexBufferHeader()
	{
		return std::get_if<PRMIndexChunkHeader>(&m_data);
	}

	const PRMVertexBufferHeader* PRMChunk::getVertexBufferHeader() const
	{
		return std::get_if<PRMVertexBufferHeader>(&m_data);
	}

	PRMVertexBufferHeader* PRMChunk::getVertexBufferHeader()
	{
		return std::get_if<PRMVertexBufferHeader>(&m_data);
	}

	void PRMChunk::recognizeChunkKindAndSaveData(Span<uint8_t> chunk, int totalChunksNr)
	{
		// Description buffer
		if (chunk.size() >= sizeof(PRMDescriptionChunkBaseHeader))
		{
			auto chunkHdr = reinterpret_cast<const PRMDescriptionChunkBaseHeader*>(&chunk[0]);
			// Helpers
			// PRM_IS_VALID_KIND - check for all known values
			// PRM_IS_VALID_PACK_TYPE - check for all known pack types
#define PRM_IS_VALID_KIND(k) (k) == 0 || (k) == 1 || (k) == 4 || (k) == 6 || (k) == 7 || (k) == 8 || (k) == 10 || (k) == 11 || (k) == 12
#define PRM_IS_VALID_PACK_TYPE(p) (p) == 0

			if (chunkHdr->ptrObjects <= totalChunksNr && PRM_IS_VALID_KIND(chunkHdr->kind) && PRM_IS_VALID_PACK_TYPE(chunkHdr->primPackType) && chunkHdr->ptrParts < totalChunksNr && chunkHdr->ptrObjects < totalChunksNr)
			{
				m_recognizedKind = PRMChunkRecognizedKind::CRK_DESCRIPTION_BUFFER;
				m_data.emplace<PRMDescriptionChunkBaseHeader>(*chunkHdr); // Copy data
				return;
			}
		}

		// Index buffer
		if (chunk.size() > 4 && (chunk.size() % 0x10) == 0)
		{
			// So, we need to check second two bytes
			auto binaryReader = ZBio::ZBinaryReader::BinaryReader(reinterpret_cast<const char*>(&chunk[0]), chunk.size());

			PRMIndexChunkHeader chunkHdr {};
			PRMIndexChunkHeader::deserialize(chunkHdr, &binaryReader);

			if (chunkHdr.indicesCount <= ((chunk.size() - 4) / 2))
			{
				m_recognizedKind = PRMChunkRecognizedKind::CRK_INDEX_BUFFER;
				m_data.emplace<PRMIndexChunkHeader>(chunkHdr);
				return;
			}
		}

		// Vertex buffer
		if (auto chunkSize = chunk.size(); (chunkSize % 0x10) == 0 || (chunkSize % 0x24) == 0 || (chunkSize % 0x28) == 0 || (chunkSize % 0x34) == 0)
		{
			PRMVertexBufferHeader vertexBufferHeader;
			vertexBufferHeader.vertexFormat = PRMVertexBufferFormat::VBF_UNKNOWN_VERTEX;

			if ((chunkSize % 0x28) == 0)
			{
				auto binaryReader = ZBio::ZBinaryReader::BinaryReader(reinterpret_cast<const char*>(&chunk[0x24]), chunk.size());
				const auto b28 = binaryReader.read<std::uint32_t, ZBio::Endianness::LE>();
				const bool is28k = (b28 == 0xCDCDCDCDu);
				if (!is28k)
				{
					m_recognizedKind = PRMChunkRecognizedKind::CRK_UNKNOWN_BUFFER;
				}
				else
				{
					vertexBufferHeader.vertexFormat = PRMVertexBufferFormat::VBF_VERTEX_28;
				}
			}

			if (m_recognizedKind == PRMChunkRecognizedKind::CRK_UNKNOWN_BUFFER)
			{
				if ((chunkSize % 0x24) == 0)
				{
					vertexBufferHeader.vertexFormat = PRMVertexBufferFormat::VBF_VERTEX_24;
				}
				else if ((chunkSize % 0x34) == 0)
				{
					vertexBufferHeader.vertexFormat = PRMVertexBufferFormat::VBF_VERTEX_34;
				}
				else if ((chunkSize % 0x10) == 0)
				{
					vertexBufferHeader.vertexFormat = PRMVertexBufferFormat::VBF_SIMPLE_VERTEX;
				}
			}

			m_recognizedKind = PRMChunkRecognizedKind::CRK_VERTEX_BUFFER;
			m_data.emplace<PRMVertexBufferHeader>(vertexBufferHeader);
			return;
		}
#undef PRM_IS_VALID_KIND
#undef PRM_IS_VALID_PACK_TYPE
	}
}