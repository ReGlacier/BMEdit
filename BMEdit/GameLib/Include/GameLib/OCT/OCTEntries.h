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
	 *
	 * Note: first entry in most cases zeroed ni vUnk4, vUnk28, vUnk34, vUnk40, unk4C, unk50
	 */
	struct OCTUnknownBlock
	{
		uint32_t unk0 { 0 }; // always 1?

		glm::mat3 vUnk4 {}; // in most cases identity matrix

		glm::vec3 vUnk28 {}; // some vector
		glm::vec3 vUnk34 { 0.f }; // another vector, Z component bigger than vUnk28
		glm::vec3 vUnk40 {}; // vector, idk

		uint32_t unk4C { 0 }; // Looks like priority or flags. In hideout first 1114, then less and decreased by 1 since second entry
		uint32_t unk50 { 0 }; // In eOUTSIDE tree always zeroed, in eINSIDE/eBOTH/eUNKNOWN in most cases 0 but sometimes > 0

		static void deserialize(OCTUnknownBlock& block, ZBio::ZBinaryReader::BinaryReader* binaryReader);
	};
}