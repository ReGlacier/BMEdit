#include <GameLib/PRM/PRMChunkDescriptor.h>
#include <GameLib/BinaryReaderSeekScope.h>
#include <GameLib/PRM/PRMBadChunkException.h>
#include <GameLib/PRM/PRMBadFile.h>
#include <GameLib/PRM/PRMReader.h>
#include <GameLib/PRM/PRMChunk.h>

#include <ZBinaryReader.hpp>


namespace gamelib::prm
{
	namespace impl
	{
		using u8 = std::uint8_t;
		using u16 = std::uint16_t;
		using u32 = std::uint32_t;

		struct V3
		{
			float x;
			float y;
			float z;
		};

		// Bounding Box
		struct BoundingBox
		{
			V3 min;
			V3 max;
		};

		// Generic PRM root entry
		struct PRMChunkHeader
		{
			u8    bone_decl_offset;
			u8    prim_pack_type;
			u16   kind;
			u16   texture_id;
			u16   unk6;
			u32   next_variation;
			u8    unk_c;
			u8    unk_d;
			u8    unk_e;
			u8    current_variation;
			u16   ptr_parts;
			u16   material_idx;
			u32   total_variations;
			u32   ptr_objects;
			u32   unk_3;
			BoundingBox bounding_box;
		};
	}

	static PRMChunkRecognizedKind recognizeChunkKind(const Span<uint8_t>& chunk, std::uint32_t totalChunksNr)
	{
		// Description buffer
		if (chunk.size() >= sizeof(impl::PRMChunkHeader))
		{
			auto chunkHdr = reinterpret_cast<const impl::PRMChunkHeader*>(&chunk[0]);
			// Helpers
			// PRM_IS_VALID_KIND - check for all known values
			// PRM_IS_VALID_PACK_TYPE - check for all known pack types
#define PRM_IS_VALID_KIND(k) (k) == 0 || (k) == 1 || (k) == 4 || (k) == 6 || (k) == 7 || (k) == 8 || (k) == 10 || (k) == 11 || (k) == 12
#define PRM_IS_VALID_PACK_TYPE(p) (p) == 0

			if (chunkHdr->ptr_objects <= totalChunksNr && PRM_IS_VALID_KIND(chunkHdr->kind) && PRM_IS_VALID_PACK_TYPE(chunkHdr->prim_pack_type) && chunkHdr->ptr_parts < totalChunksNr && chunkHdr->ptr_objects < totalChunksNr)
			{
				return PRMChunkRecognizedKind::CRK_DESCRIPTION_BUFFER;
			}
		}

		// Index buffer
		if (chunk.size() > 4 && (chunk.size() % 0x10) == 0)
		{
			// So, we need to check second two bytes
			auto binaryReader = ZBio::ZBinaryReader::BinaryReader(reinterpret_cast<const char*>(&chunk[0]), chunk.size());
			[[maybe_unused]] auto unk0 = binaryReader.read<std::uint16_t, ZBio::Endianness::LE>();
			auto indicesCount = binaryReader.read<std::uint16_t, ZBio::Endianness::LE>();

			if (indicesCount <= ((chunk.size() - 4) / 2))
			{
				return PRMChunkRecognizedKind::CRK_INDEX_BUFFER;
			}
		}

		// Vertex buffer
		if (auto chunkSize = chunk.size(); (chunkSize % 0x10) == 0 || (chunkSize % 0x24) == 0 || (chunkSize % 0x28) == 0 || (chunkSize % 0x34) == 0)
		{
			if ((chunkSize % 0x28) == 0 && (chunkSize % 0x10) != 0) // is it ok?
			{
				auto binaryReader = ZBio::ZBinaryReader::BinaryReader(reinterpret_cast<const char*>(&chunk[0x24]), chunk.size());
				const auto b28 = binaryReader.read<std::uint32_t, ZBio::Endianness::LE>();
				const bool is28k = b28 == 0xCDCDCDCDu;
				if (!is28k)
				{
					assert(false && "Bad case?");
					return PRMChunkRecognizedKind::CRK_UNKNOWN_BUFFER;
				}
			}

			return PRMChunkRecognizedKind::CRK_VERTEX_BUFFER;
		}

		return PRMChunkRecognizedKind::CRK_UNKNOWN_BUFFER;
#undef PRM_IS_VALID_KIND
#undef PRM_IS_VALID_PACK_TYPE
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
			return false;
		}

		if (m_header.countOfPrimitives >= 40960) // There are 40960 geoms max. Need to check it later
		{
			throw PRMBadFile("Possibly invalid PRM file. Game supports max 40959 unique primitives per level");
			return false;
		}

		m_chunks.reserve(m_header.countOfPrimitives);
		m_chunkDescriptors.reserve(m_header.countOfPrimitives);

		for (std::uint32_t chunkIndex = 0u; chunkIndex < m_header.countOfPrimitives; ++chunkIndex)
		{
			if (m_header.chunkOffset + (chunkIndex * PRMChunkDescriptor::kDescriptorSize) >= buffer.size())
			{
				throw PRMBadChunkException(chunkIndex);
				return false;
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

			const auto recognizedChunkKind = (chunkIndex == 0u)
			    ? PRMChunkRecognizedKind::CRK_ZERO_CHUNK
			    : recognizeChunkKind({ chunkBuffer.get(), static_cast<std::int64_t>(chunkBufferSize) }, m_header.countOfPrimitives);
			m_chunks.emplace_back(chunkIndex, std::move(chunkBuffer), chunkBufferSize, recognizedChunkKind);
		}

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