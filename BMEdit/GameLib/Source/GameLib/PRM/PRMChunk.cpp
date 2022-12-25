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

	std::size_t PRMChunk::getBufferSize() const
	{
		return m_bufferSize;
	}

	PRMChunkRecognizedKind PRMChunk::getKind() const
	{
		return m_recognizedKind;
	}

	const PRMDescriptionChunkBaseHeader* PRMChunk::getDescriptionBufferHeader() const
	{
		if (auto metaData = std::get_if<MetaData>(&m_data))
		{
			return &metaData->header;
		}

		return nullptr;
	}

	PRMDescriptionChunkBaseHeader* PRMChunk::getDescriptionBufferHeader()
	{
		if (auto metaData = std::get_if<MetaData>(&m_data))
		{
			return &metaData->header;
		}

		return nullptr;
	}

	const PRMIndexChunkHeader* PRMChunk::getIndexBufferHeader() const
	{
		if (auto indexData = std::get_if<IndexData>(&m_data))
		{
			return &indexData->header;
		}

		return nullptr;
	}

	PRMIndexChunkHeader* PRMChunk::getIndexBufferHeader()
	{
		if (auto indexData = std::get_if<IndexData>(&m_data))
		{
			return &indexData->header;
		}

		return nullptr;
	}

	const PRMVertexBufferHeader* PRMChunk::getVertexBufferHeader() const
	{
		if (auto vertexData = std::get_if<VertexData>(&m_data))
		{
			return &vertexData->header;
		}

		return nullptr;
	}

	PRMVertexBufferHeader* PRMChunk::getVertexBufferHeader()
	{
		if (auto vertexData = std::get_if<VertexData>(&m_data))
		{
			return &vertexData->header;
		}

		return nullptr;
	}

	const PRMVertexFormatDetails* PRMChunk::getVertexFormatDetails() const
	{
		if (auto vertexData = std::get_if<VertexData>(&m_data))
		{
			return &vertexData->details;
		}

		return nullptr;
	}

	PRMVertexFormatDetails* PRMChunk::getVertexFormatDetails()
	{
		if (auto vertexData = std::get_if<VertexData>(&m_data))
		{
			return &vertexData->details;
		}

		return nullptr;
	}

	bool detectVertexFormat(Span<uint8_t> chunk, PRMChunkRecognizedKind& kind, PRMVertexBufferFormat& format)
	{
		auto chunkSize = chunk.size();
		if ((chunkSize % 0x10) != 0 && (chunkSize % 0x24) != 0 && (chunkSize % 0x28) != 0 && (chunkSize % 0x34) != 0)
			return false; // Bad size

		static constexpr uint8_t s28b[4] = { 0xCD, 0xCD, 0xCD, 0xCD };

		if ((chunkSize % 0x10) == 0)
		{
			int64_t lastSegnificantBytePos = chunk.size() - 1;

			while (lastSegnificantBytePos > 1)
			{
				if (chunk[lastSegnificantBytePos] != 0x0)
					break;

				--lastSegnificantBytePos;
			}

			++lastSegnificantBytePos;

			if (lastSegnificantBytePos == chunk.size())
			{
				kind = PRMChunkRecognizedKind::CRK_VERTEX_BUFFER;
				format = PRMVertexBufferFormat::VBF_VERTEX_10;
				return true;
			}

			if ((lastSegnificantBytePos % 0x28) == 0 && std::memcmp(&chunk[0x24], &s28b[0], sizeof(s28b)) == 0)
			{
				kind = PRMChunkRecognizedKind::CRK_VERTEX_BUFFER;
				format = PRMVertexBufferFormat::VBF_VERTEX_28;
				return true;
			}

			if ((lastSegnificantBytePos % 0x24) == 0)
			{
				kind = PRMChunkRecognizedKind::CRK_VERTEX_BUFFER;
				format = PRMVertexBufferFormat::VBF_VERTEX_24;
				return true;
			}

			if ((lastSegnificantBytePos % 0x34) == 0)
			{
				kind = PRMChunkRecognizedKind::CRK_VERTEX_BUFFER;
				format = PRMVertexBufferFormat::VBF_VERTEX_34;
				return true;
			}
		}
		else if ((chunkSize % 0x28) == 0 && std::memcmp(&chunk[0x24], &s28b[0], sizeof(s28b)) == 0)
		{
			kind = PRMChunkRecognizedKind::CRK_VERTEX_BUFFER;
			format = PRMVertexBufferFormat::VBF_VERTEX_28;
			return true;
		}
		else if ((chunkSize % 0x24) == 0)
		{
			kind = PRMChunkRecognizedKind::CRK_VERTEX_BUFFER;
			format = PRMVertexBufferFormat::VBF_VERTEX_24;
			return true;
		}
		else if ((chunkSize % 0x34) == 0)
		{
			kind = PRMChunkRecognizedKind::CRK_VERTEX_BUFFER;
			format = PRMVertexBufferFormat::VBF_VERTEX_34;
			return true;
		}

		return false;
	}

	void PRMChunk::recognizeChunkKindAndSaveData(Span<uint8_t> chunk, int totalChunksNr)
	{
		// Description buffer
		if (chunk.size() >= sizeof(PRMDescriptionChunkBaseHeader))
		{
			auto binaryReader = ZBio::ZBinaryReader::BinaryReader(reinterpret_cast<const char*>(&chunk[0]), chunk.size());

			PRMDescriptionChunkBaseHeader chunkHdr;
			PRMDescriptionChunkBaseHeader::deserialize(chunkHdr, &binaryReader);

			// Helpers
			// PRM_IS_VALID_KIND - check for all known values
			// PRM_IS_VALID_PACK_TYPE - check for all known pack types
#define PRM_IS_VALID_KIND(k) (k) == 0 || (k) == 1 || (k) == 4 || (k) == 6 || (k) == 7 || (k) == 8 || (k) == 10 || (k) == 11 || (k) == 12
#define PRM_IS_VALID_PACK_TYPE(p) (p) == 0

			if (chunkHdr.ptrObjects <= totalChunksNr && PRM_IS_VALID_KIND(chunkHdr.kind) && PRM_IS_VALID_PACK_TYPE(chunkHdr.primPackType) && chunkHdr.ptrParts < totalChunksNr && chunkHdr.ptrObjects < totalChunksNr)
			{
				m_recognizedKind = PRMChunkRecognizedKind::CRK_DESCRIPTION_BUFFER;

				auto& data = m_data.emplace<MetaData>();
				data.header = chunkHdr;
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

				auto& data = m_data.emplace<IndexData>();
				data.header = chunkHdr;
				return;
			}
		}

		// Vertex buffer
		{
			PRMVertexBufferHeader vertexBufferHeader;
			vertexBufferHeader.vertexFormat = PRMVertexBufferFormat::VBF_UNKNOWN_VERTEX;

			if (detectVertexFormat(chunk, m_recognizedKind, vertexBufferHeader.vertexFormat))
			{
				auto& data = m_data.emplace<VertexData>();
				data.header = vertexBufferHeader;
				data.details = PRMVertexFormatDetails(this);
				return;
			}
		}

		// Bone description
		if (auto chunkSize = chunk.size(); (chunkSize % 0x40) == 0)
		{
			//assert(false && "Unsupported thing");
			m_recognizedKind = PRMChunkRecognizedKind::CRK_UNKNOWN_BUFFER;
			m_data.emplace<NullData>();
			return;
		}
#undef PRM_IS_VALID_KIND
#undef PRM_IS_VALID_PACK_TYPE
	}
}