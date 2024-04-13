/**
* This file contains definition of internal structure which defines information about room "eXit" segments
*/
#pragma once

#include <GameLib/Span.h>
#include <glm/vec3.hpp>
#include <cstdint>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::gms::room
{
#pragma pack(push, 1)
	/**
	 * @struct ZRoomExit
	 * @brief This structure represents all information about single room exit. See ZROOM.json properties iExitsCount and ExitOffsets
	 * @note  Total size of this structure must be 0x38 bytes for PC version (main supported game version)
	 */
	struct ZRoomExit
	{
		// Plane vertices
		glm::vec3 v0;
		glm::vec3 v1;
		glm::vec3 v2;
		glm::vec3 v3;

		// Other data
		uint32_t iRoomREF;  // Instance ID, just lookup over entities on scene to locate it
		uint8_t unk1C;
		uint8_t unk1D;
		uint8_t unkFlags; // bit#2 - always required, bit#4 - means processed remap or not (setup by engine in ZROOM::RemapRefs)
		uint8_t unk1F;

		static void deserialize(ZRoomExit& eXit, ZBio::ZBinaryReader::BinaryReader *bufBinaryReader);
		static void deserialize(ZRoomExit& eXit, const Span<uint8_t>& byteBufferSpan);
	};
#pragma pack(pop)

#pragma pack(push, 1)
	/**
	 * @struct ZRoomNeighbor
	 * @brief Describes room neighbor. Declared in BUF file. See ZROOM.json properties iNeighboursCount and NeighborsOffset
	 */
	struct ZRoomNeighbor
	{
		uint32_t rRoomREF; // Instance ID (not owner)
		uint32_t unk4;     // Means some 'amount of objects'. Maybe amount of 'neighbors'?
		uint32_t unk8;     // Some offset inside BUF. Maybe 'neighbors'?

		static void deserialize(ZRoomNeighbor& neighbor, ZBio::ZBinaryReader::BinaryReader *bufBinaryReader);
		static void deserialize(ZRoomNeighbor& neighbor, const Span<uint8_t>& byteBufferSpan);
	};
#pragma pack(pop)


	static_assert(sizeof(ZRoomExit) == 0x38);
}