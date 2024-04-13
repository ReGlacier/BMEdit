#pragma once

#include <cstdint>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::oct
{
	struct OCTHeader
	{
		uint32_t objectsOffset { 0 };
		glm::vec3 vWorldOrigin { .0f };
		float fWorldScale { .0f };

		static void deserialize(OCTHeader& header, ZBio::ZBinaryReader::BinaryReader* binaryReader);
	};

	struct OCTNode
	{
		uint16_t childCount { 0 };  // It's mask. Real count of objects could be extracted via (childCount >> 3) & 0xFFF
		uint16_t childIndex { 0 };  // It's index of NODE
		uint16_t objectIndex { 0 };

		[[nodiscard]] uint16_t getChildCount() const { return (childCount >> 3) & 0xFFF; }

		static void deserialize(OCTNode& node, ZBio::ZBinaryReader::BinaryReader* binaryReader);
	};

	struct OCTObject
	{
		uint32_t gameObjectREF { 0 };
		glm::i16vec3 vMin { 0 };
		glm::i16vec3 vMax { 0 };

		static void deserialize(OCTObject& object, ZBio::ZBinaryReader::BinaryReader* binaryReader);
	};

	/**
	 * Idk what this block contains
	 */
	struct OCTUnknownBlock
	{
		uint32_t unk0 { 0 };

		glm::mat3 vUnk4 {};
		glm::vec3 vUnk28 {};

		float unk34 { 0.f };
		float unk38 { 0.f };
		float unk3C { 0.f };

		glm::vec3 vUnk40 {};
		uint32_t unk4C { 0 };
		float unk50 { 0.f };

		static void deserialize(OCTUnknownBlock& block, ZBio::ZBinaryReader::BinaryReader* binaryReader);
	};
}