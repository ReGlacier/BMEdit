/**
 * Credits:
 * 	* 2kpr - https://github.com/glacier-modding/io_scene_blood_money/blob/libraries/BMExport/src/Prm.hpp
 */
#pragma once

#include <memory>
#include <vector>
#include <string>
#include <cstdint>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace ZBio::ZBinaryWriter
{
	class BinaryWriter;
}

namespace gamelib::prm
{
	enum class VertexFormat : uint32_t {
		VF_ERROR = 0,
		VF_10 = 0x10,
		VF_24 = 0x24,
		VF_28 = 0x28,
		VF_34 = 0x34
	};

#pragma pack(push, 1)   // TODO: Need to use some sort of macro to make this place cross-compiler supportable
	struct PrmFile;

	struct Index
	{
		uint16_t a = 0;
		uint16_t b = 0;
		uint16_t c = 0;

		static void deserialize(Index& index, ZBio::ZBinaryReader::BinaryReader* binaryReader);
	};

	struct BoundingBox
	{
		glm::vec3 vMin;
		glm::vec3 vMax;

		static void deserialize(BoundingBox& boundingBox, ZBio::ZBinaryReader::BinaryReader* binaryReader);
	};

	struct Mesh
	{
		uint8_t boneDecl = 0;
		uint8_t packType = 0;
		uint16_t kind = 0;
		uint16_t textureId = 0;
		uint16_t unk6 = 0;
		uint32_t nextVariation = 0;
		uint8_t unkC = 0;
		uint8_t unkD = 0;
		uint8_t lod = 0;
		uint16_t material_id = 0;
		uint8_t variationId = 0;
		int32_t diffuse_id = 0;
		int32_t normal_id = 0;
		int32_t specular_id = 0;
		uint16_t trianglesCount = 0;
		std::vector<glm::vec3> vertices {};
		std::vector<Index> indices {};
		std::vector<glm::vec2> uvs {};
		VertexFormat vertexFormat { VertexFormat::VF_ERROR };

		static void deserialize(Mesh& mesh, ZBio::ZBinaryReader::BinaryReader* binaryReader, const PrmFile& prmFile);
	};

	struct Model
	{
		uint32_t chunk = 0;
		BoundingBox boundingBox {};
		std::vector<Mesh> meshes {};
	};

	struct Chunk
	{
		std::unique_ptr<std::uint8_t[]> data { nullptr };
		bool is_model = false;
		uint32_t model = 0;
	};

	struct Entry
	{
		uint32_t offset = 0;
		uint32_t size = 0;
		uint32_t type = 0;
		uint32_t pad = 0;

		static void deserialize(Entry& entry, ZBio::ZBinaryReader::BinaryReader* binaryReader);
	};

	struct Header
	{
		uint32_t table_offset = 0;
		uint32_t table_count = 0;
		uint32_t table_offset2 = 0;
		uint32_t zeroed = 0;

		static void deserialize(Header& header, ZBio::ZBinaryReader::BinaryReader* binaryReader);
	};

	struct PrmFile
	{
		Header header;
		std::vector<Entry> entries;
		std::vector<Chunk> chunks;
		std::vector<Model> models;
	};

	/// After 02.02.2025: More detailed & correct structures below
//	enum EPrimPackType : uint16_t
//	{
//		PTSTRIP = 0,
//		PTSTRIPBONES = 1,
//		PTSPRITES = 2,
//		// #3 MISSING
//		PACKTYPEBACKDROP = 4,
//		PTSTRIPBONESV = 5,
//		PTDOT3STRIP = 6,
//		PTOBJECTHEADER = 7,
//		PTMESH = 8,
//		// #9 MISSING
//		// #10 MISSING
//		PTWATERPATCH = 11,
//		PTLIGHT = 12,
//	};

	struct STransformation {
		using mat3 = float[9];
		using vec3 = float[3];

		mat3 mTransform { 1.f, 0.f, 0.f,
		                  0.f, 1.f, 0.f,
		                  0.f, 0.f, 1.f };
		vec3 vTransform { 0.f, 0.f, 0.f };

		static void deserialize(STransformation& transform, ZBio::ZBinaryReader::BinaryReader* binaryReader);
		static void serialize(const STransformation& transform, ZBio::ZBinaryWriter::BinaryWriter* binaryWriter);
	};

	struct SHandleTableEntry {
		uint32_t lOffset { 0 };
		uint32_t lSize { 0 };
		uint32_t lRefCount { 0 };
		uint32_t lPad { 0 };

		static void deserialize(SHandleTableEntry& entry, ZBio::ZBinaryReader::BinaryReader* binaryReader);
		static void serialize(const SHandleTableEntry& entry, ZBio::ZBinaryWriter::BinaryWriter* binaryWriter);
	};

	enum EPrimType : uint16_t
	{
		PTSTRIP = 0,
		PTSTRIPBONES = 1,
		PTSPRITES = 2,
		// #3 MISSING
		// #4 MISSING
		PTSTRIPBONESV = 5,
		PTDOT3STRIP = 6,
		PTOBJECTHEADER = 7,
		PTMESH = 8,
		// #9 MISSING
		// #10 MISSING
		PTWATERPATCH = 11,
		PTLIGHT = 12,
	};

	struct SPrimHeader {
		uint8_t  lDrawDestination { 0 };
		uint8_t  lPackType { 0 };  // Known value 4 (PACKTYPEBACKDROP)
		union {
			uint16_t lType { 0 };
			EPrimType eType;
		} Type;

		static void deserialize(SPrimHeader& prims, ZBio::ZBinaryReader::BinaryReader* binaryReader);
		static void serialize(const SPrimHeader& prims, ZBio::ZBinaryWriter::BinaryWriter* binaryWriter);
	};

	struct SPrims {
		SPrimHeader header {};
		uint16_t lTextureId { 0 };
		uint16_t lDrawEntryId { 0 };
		int32_t  lNextPrim { 0 };

		static void deserialize(SPrims& prims, ZBio::ZBinaryReader::BinaryReader* binaryReader);
		static void serialize(const SPrims& prims, ZBio::ZBinaryWriter::BinaryWriter* binaryWriter);
	};

	struct SPrimObjectHeader {
		SPrims base {};
		int32_t  lPropertyFlags { 0 };
		int32_t  lPropertyData { 0 };
		int32_t  lNumObjects { 0 };
		int32_t  lObjectTable { 0 };
		int32_t  lColiId { 0 };
		float    vMin[3] { 0.f };
		float    vMax[3] { 0.f };
		int32_t  lPlanes { 0 };

		static void deserialize(SPrimObjectHeader& header, ZBio::ZBinaryReader::BinaryReader* binaryReader);
		static void serialize(const SPrimObjectHeader& header, ZBio::ZBinaryWriter::BinaryWriter* binaryWriter);
	};

	struct SPrimObject {
		SPrims base {};
		uint8_t  lSubType { 0 };
		uint8_t  lProperties { 0 };
		uint8_t  lLODMask { 0 };
		uint8_t  lVariantId { 0 };
		uint8_t  lNumInstances { 0 };
		uint8_t  lPad { 0 };
		uint16_t lMaterialId { 0 };
		int32_t  lColiBits { 0 };
		int32_t  lWireColor { 0 };
		int32_t  lDrawMode { 0 };
		int32_t  lTransformations { 0 };
		int32_t  lExtraData { 0 };

		static void deserialize(SPrimObject& object, ZBio::ZBinaryReader::BinaryReader* binaryReader);
		static void serialize(const SPrimObject& object, ZBio::ZBinaryWriter::BinaryWriter* binaryWriter);
	};

	struct SPrimMesh {
		SPrimObject object {};
		int32_t  lSubMeshTable { 0 };
		int32_t  lNumFrames { 0 };
		uint16_t lFrameStart { 0 };
		uint16_t lFrameStep { 0 };
		int32_t  lTrisPerStripColor { 0 };

		static void deserialize(SPrimMesh& mesh, ZBio::ZBinaryReader::BinaryReader* binaryReader);
		static void serialize(const SPrimMesh& mesh, ZBio::ZBinaryWriter::BinaryWriter* binaryWriter);
	};

	struct SPrimHeaderStrip {
		SPrimHeader header {};
		uint8_t  Plane0[16] { 0 };
		uint8_t  Plane1[16] { 0 };
		float    vMax[3] { 0.f };
		float    vMin[3] { 0.f };

		static void deserialize(SPrimHeaderStrip& mesh, ZBio::ZBinaryReader::BinaryReader* binaryReader);
		static void serialize(const SPrimHeaderStrip& mesh, ZBio::ZBinaryWriter::BinaryWriter* binaryWriter);
	};

#pragma pack(pop)
}