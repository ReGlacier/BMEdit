#include <GameLib/PRM/PRMReader.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>


namespace gamelib
{
	PRMReader::PRMReader() = default;

	bool PRMReader::parse(const std::uint8_t *pBuffer, std::size_t iBufferSize)
	{
		if (!pBuffer || !iBufferSize)
			return false;

		// Initialize global reader
		ZBio::ZBinaryReader::BinaryReader reader { reinterpret_cast<const char*>(pBuffer), static_cast<int64_t>(iBufferSize) };

		// Read header
		prm::Header::deserialize(m_file.header, &reader);

		// Seek & read entries
		{
			ZBio::ZBinaryReader::BinaryReader entriesReader { reinterpret_cast<const char*>(pBuffer), static_cast<int64_t>(iBufferSize) };
			entriesReader.seek(m_file.header.table_offset);
			m_file.entries.reserve(m_file.header.table_count); // actually, it's not a table count, it's amount of chunks.

			for (int i = 0; i < m_file.header.table_count; i++)
			{
				prm::Entry& entry = m_file.entries.emplace_back();
				prm::Entry::deserialize(entry, &entriesReader);
			}

			m_file.chunks.reserve(m_file.header.table_count);
		}

		// Prepare chunks
		{
			for (int i = 0; i < m_file.header.table_count; i++)
			{
				ZBio::ZBinaryReader::BinaryReader chunksReader { reinterpret_cast<const char*>(pBuffer), static_cast<int64_t>(iBufferSize) };
				chunksReader.seek(m_file.entries[i].offset);

				prm::Chunk& chunk = m_file.chunks.emplace_back();
				chunk.data = std::make_unique<std::uint8_t[]>(m_file.entries[i].size);

				// TODO: Need to check that data is valid (malloc is ok)
				chunksReader.read<std::uint8_t, ZBio::Endianness::LE>(chunk.data.get(), m_file.entries[i].size);

				// TODO: Need to refactor and use former header for chunk buffer instead of cropping few bytes (will fix later)
				// TODO: Need to use proper way to read bytes (endianness)
				if (m_file.entries[i].size == 0x40 && *reinterpret_cast<std::uint32_t*>(chunk.data.get()) == 0x70100)
				{
					chunk.is_model = true;
					chunk.model = static_cast<uint32_t>(m_file.models.size());

					prm::Model& newMdl = m_file.models.emplace_back();
					newMdl.chunk = i;
				}
			}
		}

		// Read models
		{
			for (prm::Model& model : m_file.models)
			{
				ZBio::ZBinaryReader::BinaryReader modelReader {
					reinterpret_cast<const char*>(m_file.chunks[model.chunk].data.get()),
				    static_cast<int64_t>(m_file.entries[model.chunk].size)
				};

				modelReader.seek(0x14);
				uint32_t meshCount = 0, meshTable = 0;

				meshCount = modelReader.read<uint32_t, ZBio::Endianness::LE>();
				meshTable = modelReader.read<uint32_t, ZBio::Endianness::LE>();

				// Read mesh table
				ZBio::ZBinaryReader::BinaryReader meshTableReader {
				    reinterpret_cast<const char*>(m_file.chunks[meshTable].data.get()),
				    static_cast<int64_t>(m_file.entries[meshTable].size)
				};

				for (uint32_t i = 0; i < meshCount; i++)
				{
					// Read mesh chunk index
					uint32_t currentMeshChunkIdx = meshTableReader.read<uint32_t, ZBio::Endianness::LE>(); // NOLINT(*-use-auto)

					// And read mesh itself
					ZBio::ZBinaryReader::BinaryReader meshEntryReader {
					    reinterpret_cast<const char*>(m_file.chunks[currentMeshChunkIdx].data.get()),
					    static_cast<int64_t>(m_file.entries[currentMeshChunkIdx].size)
					};

					// Read mesh
					prm::Mesh currentMesh {};
					prm::Mesh::deserialize(currentMesh, &meshEntryReader, m_file);

					if (currentMesh.lod & (uint8_t)1 == (uint8_t)1)
					{
						// Save mesh
						model.meshes.emplace_back(std::move(currentMesh));
					}
				}
			}
		}

		return true;
	}

	prm::PrmFile&& PRMReader::takePrimitives()
	{
		return std::move(m_file);
	}
}