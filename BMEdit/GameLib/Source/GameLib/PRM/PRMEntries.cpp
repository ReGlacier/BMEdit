#include <GameLib/PRM/PRMEntries.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>
#include <ZBinaryWriter.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace gamelib::prm
{
	void Index::deserialize(Index& index, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		index.a = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		index.b = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		index.c = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
	}

	void BoundingBox::deserialize(BoundingBox& boundingBox, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		boundingBox.vMin.x = binaryReader->read<float, ZBio::Endianness::LE>();
		boundingBox.vMin.y = binaryReader->read<float, ZBio::Endianness::LE>();
		boundingBox.vMin.z = binaryReader->read<float, ZBio::Endianness::LE>();
		boundingBox.vMax.x = binaryReader->read<float, ZBio::Endianness::LE>();
		boundingBox.vMax.y = binaryReader->read<float, ZBio::Endianness::LE>();
		boundingBox.vMax.z = binaryReader->read<float, ZBio::Endianness::LE>();
	}

	void Mesh::deserialize(Mesh& mesh, ZBio::ZBinaryReader::BinaryReader* binaryReader, const PrmFile& prmFile)
	{
		mesh.boneDecl = binaryReader->read<uint8_t, ZBio::Endianness::LE>();
		mesh.packType = binaryReader->read<uint8_t, ZBio::Endianness::LE>();
		mesh.kind = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		mesh.textureId = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		mesh.unk6 = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		mesh.nextVariation = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		mesh.unkC = binaryReader->read<uint8_t, ZBio::Endianness::LE>();
		mesh.unkD = binaryReader->read<uint8_t, ZBio::Endianness::LE>();

		assert(binaryReader->tell() == 0xE && "Bad offset");
		if (binaryReader->tell() != 0xE)
			return;

		mesh.lod = binaryReader->read<uint8_t, ZBio::Endianness::LE>();

		if (mesh.lod & (uint8_t)1 == (uint8_t)1)
		{
			// Read mesh variation index
			mesh.variationId = binaryReader->read<uint8_t, ZBio::Endianness::LE>();

			// Seed another 2 bytes?
			ZBioHelpers::seekBy(binaryReader, 0x2);

			// Read material id
			mesh.material_id = binaryReader->read<uint16_t, ZBio::Endianness::LE>();

			// Jump next
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
				mesh.trianglesCount = trianglesCount;

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

				assert(vertexSize == 0x10 || vertexSize == 0x24 || vertexSize == 0x28 || vertexSize == 0x34);
				if (vertexSize == 0x10 || vertexSize == 0x24 || vertexSize == 0x28 || vertexSize == 0x34)
				{
					mesh.vertexFormat = static_cast<VertexFormat>(vertexSize);

					switch (mesh.vertexFormat)
					{
						case VertexFormat::VF_10:
						{
						    for (uint32_t j = 0; j < vertexCount; j++)
						    {
							    glm::vec3& vertex = mesh.vertices.emplace_back();
							    vertexReader.read<float, ZBio::Endianness::LE>(glm::value_ptr(vertex), 3);

							    uint8_t l[4] { 0, 0, 0, 0 };
							    vertexReader.read<uint8_t, ZBio::Endianness::LE>(&l[0], 4);
						    }
					    }
					    break;
						case VertexFormat::VF_24:
					    {
						    for (uint32_t j = 0; j < vertexCount; j++)
						    {
							    glm::vec3& vertex = mesh.vertices.emplace_back();
							    vertexReader.read<float, ZBio::Endianness::LE>(glm::value_ptr(vertex), 3);

							    // Skip another 0x10 useful info
							    // TODO: Fix this!
							    ZBioHelpers::seekBy(&vertexReader, 0x10);

							    // Read UVs
							    glm::vec2& uv = mesh.uvs.emplace_back();
							    vertexReader.read<float, ZBio::Endianness::LE>(glm::value_ptr(uv), 2);
						    }
					    }
					    break;
						case VertexFormat::VF_28:
					    {
						    for (uint32_t j = 0; j < vertexCount; j++)
						    {
							    glm::vec3& vertex = mesh.vertices.emplace_back();
							    vertexReader.read<float, ZBio::Endianness::LE>(glm::value_ptr(vertex), 3);

							    // Skip another 0x10 useful info
							    // TODO: Fix this!
							    ZBioHelpers::seekBy(&vertexReader, 0x8);

							    // Read UVs
							    glm::vec2& uv = mesh.uvs.emplace_back();
							    vertexReader.read<float, ZBio::Endianness::LE>(glm::value_ptr(uv), 2);

							    // Another seek
							    // TODO: Fix this!
							    ZBioHelpers::seekBy(&vertexReader, 0xC);
						    }
					    }
					    break;
						case VertexFormat::VF_34:
					    {
						    for (uint32_t j = 0; j < vertexCount; j++)
						    {
							    glm::vec3& vertex = mesh.vertices.emplace_back();
							    vertexReader.read<float, ZBio::Endianness::LE>(glm::value_ptr(vertex), 3);

							    // TODO: Fix this!
							    ZBioHelpers::seekBy(&vertexReader, 0x18);

							    glm::vec2& uv = mesh.uvs.emplace_back();
							    vertexReader.read<float, ZBio::Endianness::LE>(glm::value_ptr(uv), 2);

							    // TODO: Fix this
							    ZBioHelpers::seekBy(&vertexReader, 0x8);
						    }
					    }
					    break;
					    default:
						    assert(false && "Unsupported format");
						    break;
					}

					// Store triangle indices
					for (uint32_t j = 0; j < mesh.trianglesCount / 3; j++)
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

	// After 02.02.2025: More detailed & correct structures below
	void STransformation::deserialize(STransformation& transform, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		binaryReader->read<float, ZBio::Endianness::LE>(&transform.mTransform[0], 9);
		binaryReader->read<float, ZBio::Endianness::LE>(&transform.vTransform[0], 3);
	}

	void STransformation::serialize(const STransformation& transform, ZBio::ZBinaryWriter::BinaryWriter* binaryWriter)
	{
		binaryWriter->write<float, ZBio::Endianness::LE>(&transform.mTransform[0], 9);
		binaryWriter->write<float, ZBio::Endianness::LE>(&transform.vTransform[0], 3);
	}

	void SHandleTableEntry::deserialize(SHandleTableEntry& entry, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		entry.lOffset = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		entry.lSize = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		entry.lRefCount = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		entry.lPad = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
	}

	void SHandleTableEntry::serialize(const SHandleTableEntry& entry, ZBio::ZBinaryWriter::BinaryWriter* binaryWriter)
	{
		binaryWriter->write<uint32_t, ZBio::Endianness::LE>(entry.lOffset);
		binaryWriter->write<uint32_t, ZBio::Endianness::LE>(entry.lSize);
		binaryWriter->write<uint32_t, ZBio::Endianness::LE>(entry.lRefCount);
		binaryWriter->write<uint32_t, ZBio::Endianness::LE>(entry.lPad);
	}

	void SPrimHeader::deserialize(SPrimHeader& prims, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		prims.lDrawDestination = binaryReader->read<uint8_t, ZBio::Endianness::LE>();
		prims.lPackType = binaryReader->read<uint8_t, ZBio::Endianness::LE>();
		prims.lType = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
	}

	void SPrimHeader::serialize(const SPrimHeader& prims, ZBio::ZBinaryWriter::BinaryWriter* binaryWriter)
	{
		binaryWriter->write<uint8_t, ZBio::Endianness::LE>(prims.lDrawDestination);
		binaryWriter->write<uint8_t, ZBio::Endianness::LE>(prims.lPackType);
		binaryWriter->write<uint16_t, ZBio::Endianness::LE>(prims.lType);
	}

	void SPrims::deserialize(SPrims& prims, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		SPrimHeader::deserialize(prims.header, binaryReader);
		prims.lTextureId = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		prims.lDrawEntryId = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		prims.lNextPrim = binaryReader->read<int32_t, ZBio::Endianness::LE>();
	}

	void SPrims::serialize(const SPrims& prims, ZBio::ZBinaryWriter::BinaryWriter* binaryWriter)
	{
		SPrimHeader::serialize(prims.header, binaryWriter);
		binaryWriter->write<uint16_t, ZBio::Endianness::LE>(prims.lTextureId);
		binaryWriter->write<uint16_t, ZBio::Endianness::LE>(prims.lDrawEntryId);
		binaryWriter->write<int32_t, ZBio::Endianness::LE>(prims.lNextPrim);
	}

	void SPrimObjectHeader::deserialize(SPrimObjectHeader& header, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		SPrims::deserialize(header.base, binaryReader);
		header.lPropertyFlags = binaryReader->read<int32_t, ZBio::Endianness::LE>();
		header.lPropertyData = binaryReader->read<int32_t, ZBio::Endianness::LE>();
		header.lNumObjects = binaryReader->read<int32_t, ZBio::Endianness::LE>();
		header.lObjectTable = binaryReader->read<int32_t, ZBio::Endianness::LE>();
		header.lColiId = binaryReader->read<int32_t, ZBio::Endianness::LE>();
		binaryReader->read<float, ZBio::Endianness::LE>(&header.vMin[0], 3);
		binaryReader->read<float, ZBio::Endianness::LE>(&header.vMax[0], 3);
		header.lPlanes = binaryReader->read<int32_t, ZBio::Endianness::LE>();
	}

	void SPrimObjectHeader::serialize(const SPrimObjectHeader& header, ZBio::ZBinaryWriter::BinaryWriter* binaryWriter)
	{
		SPrims::serialize(header.base, binaryWriter);
		binaryWriter->write<int32_t, ZBio::Endianness::LE>(header.lPropertyFlags);
		binaryWriter->write<int32_t, ZBio::Endianness::LE>(header.lPropertyData);
		binaryWriter->write<int32_t, ZBio::Endianness::LE>(header.lNumObjects);
		binaryWriter->write<int32_t, ZBio::Endianness::LE>(header.lObjectTable);
		binaryWriter->write<int32_t, ZBio::Endianness::LE>(header.lColiId);
		binaryWriter->write<float,   ZBio::Endianness::LE>(&header.vMin[0], 3);
		binaryWriter->write<float,   ZBio::Endianness::LE>(&header.vMax[0], 3);
		binaryWriter->write<int32_t, ZBio::Endianness::LE>(header.lPlanes);
	}

	void SPrimObject::deserialize(SPrimObject& object, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		SPrims::deserialize(object.base, binaryReader);
		object.lSubType         = binaryReader->read<uint8_t,  ZBio::Endianness::LE>();
		object.lProperties      = binaryReader->read<uint8_t,  ZBio::Endianness::LE>();
		object.lLODMask         = binaryReader->read<uint8_t,  ZBio::Endianness::LE>();
		object.lVariantId       = binaryReader->read<uint8_t,  ZBio::Endianness::LE>();
		object.lNumInstances    = binaryReader->read<uint8_t,  ZBio::Endianness::LE>();
		object.lPad             = binaryReader->read<uint8_t,  ZBio::Endianness::LE>();
		object.lMaterialId      = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		object.lColiBits        = binaryReader->read<int32_t,  ZBio::Endianness::LE>();
		object.lWireColor       = binaryReader->read<int32_t,  ZBio::Endianness::LE>();
		object.lDrawMode        = binaryReader->read<int32_t,  ZBio::Endianness::LE>();
		object.lTransformations = binaryReader->read<int32_t,  ZBio::Endianness::LE>();
		object.lExtraData       = binaryReader->read<int32_t,  ZBio::Endianness::LE>();
	}

	void SPrimObject::serialize(const SPrimObject& object, ZBio::ZBinaryWriter::BinaryWriter* binaryWriter)
	{
		SPrims::serialize(object.base, binaryWriter);
		binaryWriter->write<uint8_t,  ZBio::Endianness::LE>(object.lSubType);
		binaryWriter->write<uint8_t,  ZBio::Endianness::LE>(object.lProperties);
		binaryWriter->write<uint8_t,  ZBio::Endianness::LE>(object.lLODMask);
		binaryWriter->write<uint8_t,  ZBio::Endianness::LE>(object.lVariantId);
		binaryWriter->write<uint8_t,  ZBio::Endianness::LE>(object.lNumInstances);
		binaryWriter->write<uint8_t,  ZBio::Endianness::LE>(object.lPad);
		binaryWriter->write<uint16_t, ZBio::Endianness::LE>(object.lMaterialId);
		binaryWriter->write<int32_t,  ZBio::Endianness::LE>(object.lColiBits);
		binaryWriter->write<int32_t,  ZBio::Endianness::LE>(object.lWireColor);
		binaryWriter->write<int32_t,  ZBio::Endianness::LE>(object.lDrawMode);
		binaryWriter->write<int32_t,  ZBio::Endianness::LE>(object.lTransformations);
		binaryWriter->write<int32_t,  ZBio::Endianness::LE>(object.lExtraData);
	}

	void SPrimMesh::deserialize(SPrimMesh& mesh, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		SPrimObject::deserialize(mesh.object, binaryReader);
		mesh.lSubMeshTable = binaryReader->read<int32_t, ZBio::Endianness::LE>();
		mesh.lNumFrames = binaryReader->read<int32_t, ZBio::Endianness::LE>();
		mesh.lFrameStart = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		mesh.lFrameStep = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		mesh.lTrisPerStripColor = binaryReader->read<int32_t, ZBio::Endianness::LE>();
	}

	void SPrimMesh::serialize(const SPrimMesh& mesh, ZBio::ZBinaryWriter::BinaryWriter* binaryWriter)
	{
		SPrimObject::serialize(mesh.object, binaryWriter);
		binaryWriter->write<int32_t, ZBio::Endianness::LE>(mesh.lSubMeshTable);
		binaryWriter->write<int32_t, ZBio::Endianness::LE>(mesh.lNumFrames);
		binaryWriter->write<uint16_t, ZBio::Endianness::LE>(mesh.lFrameStart);
		binaryWriter->write<uint16_t, ZBio::Endianness::LE>(mesh.lFrameStep);
		binaryWriter->write<int32_t, ZBio::Endianness::LE>(mesh.lTrisPerStripColor);
	}

	void SPrimHeaderStrip::deserialize(SPrimHeaderStrip& mesh, ZBio::ZBinaryReader::BinaryReader* binaryReader)
	{
		SPrimHeader::deserialize(mesh.header, binaryReader);
		binaryReader->read<uint8_t, ZBio::Endianness::LE>(&mesh.Plane0[0], 16);
		binaryReader->read<uint8_t, ZBio::Endianness::LE>(&mesh.Plane1[0], 16);
		binaryReader->read<float, ZBio::Endianness::LE>(&mesh.vMax[0], 3);
		binaryReader->read<float, ZBio::Endianness::LE>(&mesh.vMin[0], 3);
	}

	void SPrimHeaderStrip::serialize(const SPrimHeaderStrip& mesh, ZBio::ZBinaryWriter::BinaryWriter* binaryWriter)
	{
		SPrimHeader::serialize(mesh.header, binaryWriter);
		binaryWriter->write<uint8_t, ZBio::Endianness::LE>(&mesh.Plane0[0], 16);
		binaryWriter->write<uint8_t, ZBio::Endianness::LE>(&mesh.Plane1[0], 16);
		binaryWriter->write<float, ZBio::Endianness::LE>(&mesh.vMax[0], 3);
		binaryWriter->write<float, ZBio::Endianness::LE>(&mesh.vMin[0], 3);
	}
}