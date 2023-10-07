#include <GameLib/PRM/PRMEntries.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>


namespace gamelib::prm
{
	void Index::deserialize(Index& index, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		index.a = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		index.b = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		index.c = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
	}

	void Mesh::deserialize(Mesh& mesh, ZBio::ZBinaryReader::BinaryReader* binaryReader, const PrmFile& prmFile)
	{
		// Skip header?
		binaryReader->seek(0xE);

		mesh.lod = binaryReader->read<uint8_t, ZBio::Endianness::LE>();

		if ((mesh.lod & static_cast<uint8_t>(1)) == static_cast<uint8_t>(1))
		{
			// Seed another 3 bytes?
			ZBioHelpers::seekBy(binaryReader, 0x3);

			// Read material id
			mesh.material_id = binaryReader->read<uint16_t, ZBio::Endianness::LE>();

			// Another seek
			ZBioHelpers::seekBy(binaryReader, 0x14);

			uint32_t meshDescriptionChunk = 0;

			{
				uint32_t meshDescriptionPointerChunk = 0;

				// Read description chunk index
				meshDescriptionPointerChunk = binaryReader->read<uint32_t, ZBio::Endianness::LE>();

				if (meshDescriptionPointerChunk >= prmFile.chunks.size())
				{
					// Invalid chunk? Or not?
					// TODO: Weird case, need investigate it later
					return;
				}

				// Read description
				ZBio::ZBinaryReader::BinaryReader modelDescriptionReader {
				    reinterpret_cast<const char*>(prmFile.chunks[meshDescriptionPointerChunk].data.get()),
				    static_cast<int64_t>(prmFile.entries[meshDescriptionPointerChunk].size)
				};

				// Instead of meshDescriptionPointerChunk this value pointed by meshDescriptionPointerChunk (another IOI shit code, who cares?)
				meshDescriptionChunk = modelDescriptionReader.read<uint32_t, ZBio::Endianness::LE>();

				if (meshDescriptionChunk >= prmFile.chunks.size())
				{
					// Invalid chunk? Invalid wtf? IOI!!111111
					return;
				}
			}

			// Now we almost ready to read model description (rly?)
			ZBio::ZBinaryReader::BinaryReader meshDescriptionReader {
			    reinterpret_cast<const char*>(prmFile.chunks[meshDescriptionChunk].data.get()),
			    static_cast<int64_t>(prmFile.entries[meshDescriptionChunk].size)
			};

			uint32_t vertexCount = 0, vertexChunk = 0, trianglesChunk = 0;

			vertexCount = meshDescriptionReader.read<uint32_t, ZBio::Endianness::LE>();
			vertexChunk = meshDescriptionReader.read<uint32_t, ZBio::Endianness::LE>();

			// Another seek (rly?)
			ZBioHelpers::seekBy(&meshDescriptionReader, 0x4);

			trianglesChunk = meshDescriptionReader.read<uint32_t, ZBio::Endianness::LE>();

			// Check that we have something valid here
			if (vertexCount != 0 && vertexChunk != 0 && trianglesChunk != 0)
			{
				// And another one reader
				ZBio::ZBinaryReader::BinaryReader trianglesReader {
					reinterpret_cast<const char*>(prmFile.chunks[trianglesChunk].data.get()),
					static_cast<int64_t>(prmFile.entries[trianglesChunk].size)
				};

				// Skip first 2 bytes
				ZBioHelpers::seekBy(&trianglesReader, 0x2);

				uint16_t trianglesCount = 0;
				trianglesCount = trianglesReader.read<uint16_t, ZBio::Endianness::LE>();

				// Magic (rly?)
				uint32_t vertexSize = 0;
				vertexSize = static_cast<uint32_t>(prmFile.entries[vertexChunk].size / vertexCount);
				vertexSize -= vertexSize % 4;

				if (vertexSize != 0x28 && vertexSize % 0x28 == 0)
				{
					vertexCount *= vertexSize / 0x28;
					vertexSize = 0x28;
				}

				// And vertex reader
				ZBio::ZBinaryReader::BinaryReader vertexReader {
				    reinterpret_cast<const char*>(prmFile.chunks[vertexChunk].data.get()),
				    static_cast<int64_t>(prmFile.entries[vertexChunk].size)
				};

				enum VertexFormat : uint32_t {
					VF_10 = 0x10,
					VF_24 = 0x24,
					VF_28 = 0x28,
					VF_34 = 0x34
				};

				assert(vertexSize == 0x10 || vertexSize == 0x24 || vertexSize == 0x28 || vertexSize == 0x34);
				if (vertexSize == 0x10 || vertexSize == 0x24 || vertexSize == 0x28 || vertexSize == 0x34)
				{
					VertexFormat format = static_cast<VertexFormat>(vertexSize); // NOLINT(*-use-auto)

					switch (format)
					{
						case VF_10:
						{
						    for (uint32_t j = 0; j < vertexCount; j++)
						    {
							    glm::vec3& vertex = mesh.vertices.emplace_back();
							    vertex.x = vertexReader.read<float, ZBio::Endianness::LE>();
							    vertex.y = vertexReader.read<float, ZBio::Endianness::LE>();
							    vertex.z = vertexReader.read<float, ZBio::Endianness::LE>();

							    // Skip 4 bytes (it's some sort of data, but ignored by us)
							    // TODO: Fix this!
							    ZBioHelpers::seekBy(&vertexReader, 0x4);
						    }
					    }
					    break;
						case VF_24:
					    {
						    for (uint32_t j = 0; j < vertexCount; j++)
						    {
							    glm::vec3& vertex = mesh.vertices.emplace_back();
							    vertex.x = vertexReader.read<float, ZBio::Endianness::LE>();
							    vertex.y = vertexReader.read<float, ZBio::Endianness::LE>();
							    vertex.z = vertexReader.read<float, ZBio::Endianness::LE>();

							    // Skip another 0x10 useful info
							    // TODO: Fix this!
							    ZBioHelpers::seekBy(&vertexReader, 0x10);

							    // Read UVs
							    glm::vec2& uv = mesh.uvs.emplace_back();
							    uv.x = vertexReader.read<float, ZBio::Endianness::LE>();
							    uv.y = 1.f - vertexReader.read<float, ZBio::Endianness::LE>();
						    }
					    }
					    break;
						case VF_28:
					    {
						    for (uint32_t j = 0; j < vertexCount; j++)
						    {
							    glm::vec3& vertex = mesh.vertices.emplace_back();
							    vertex.x = vertexReader.read<float, ZBio::Endianness::LE>();
							    vertex.y = vertexReader.read<float, ZBio::Endianness::LE>();
							    vertex.z = vertexReader.read<float, ZBio::Endianness::LE>();

							    // Skip another 0x10 useful info
							    // TODO: Fix this!
							    ZBioHelpers::seekBy(&vertexReader, 0x8);

							    // Read UVs
							    glm::vec2& uv = mesh.uvs.emplace_back();
							    uv.x = vertexReader.read<float, ZBio::Endianness::LE>();
							    uv.y = 1.f - vertexReader.read<float, ZBio::Endianness::LE>();

							    // Another seek
							    // TODO: Fix this!
							    ZBioHelpers::seekBy(&vertexReader, 0xC);
						    }
					    }
					    break;
						case VF_34:
					    {
						    glm::vec3& vertex = mesh.vertices.emplace_back();
						    vertex.x = vertexReader.read<float, ZBio::Endianness::LE>();
						    vertex.y = vertexReader.read<float, ZBio::Endianness::LE>();
						    vertex.z = vertexReader.read<float, ZBio::Endianness::LE>();

						    // TODO: Fix this!
						    ZBioHelpers::seekBy(&vertexReader, 0x18);

						    glm::vec2& uv = mesh.uvs.emplace_back();
						    uv.x = vertexReader.read<float, ZBio::Endianness::LE>();
						    uv.y = 1.f - vertexReader.read<float, ZBio::Endianness::LE>();

						    // TODO: Fix this
						    ZBioHelpers::seekBy(&vertexReader, 0x8);
					    }
					    break;
					}

					// Store triangle indices
					for (uint32_t j = 0; j < trianglesCount / 3; j++)
					{
						prm::Index& index = mesh.indices.emplace_back();
						prm::Index::deserialize(index, &trianglesReader);
					}
				}
			}
		}
	}

	void Entry::deserialize(Entry& entry, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		entry.offset = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		entry.size = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		entry.type = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		entry.pad = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
	}

	void Header::deserialize(Header& header, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		header.table_offset = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		header.table_count = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		header.table_offset2 = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		header.zeroed = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
	}
}