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

namespace gamelib::prm
{
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
		uint16_t textureId = 0;
		uint8_t lod = 0;
		uint16_t material_id = 0;
		int32_t diffuse_id = 0;
		int32_t normal_id = 0;
		int32_t specular_id = 0;
		std::vector<glm::vec3> vertices {};
		std::vector<Index> indices {};
		std::vector<glm::vec2> uvs {};
		//BoundingBox boundingBox {};

		static void deserialize(Mesh& mesh, ZBio::ZBinaryReader::BinaryReader* binaryReader, const PrmFile& prmFile);
	};

	struct Model
	{
		uint32_t chunk = 0;
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
#pragma pack(pop)
}