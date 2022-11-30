#pragma once

#include <GameLib/BoundingBox.h>
#include <GameLib/Vector3.h>
#include <cstdint>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::prm
{
	struct PRMDescriptionChunkBaseHeader
	{
		std::uint8_t    boneDeclOffset { 0 };
		std::uint8_t    primPackType { 0 };
		std::uint16_t   kind { 0 };
		std::uint16_t   textureId { 0 };
		std::uint16_t   unk6 { 0 };
		std::uint32_t   nextVariation { 0 };
		std::uint8_t    unkC { 0 };
		std::uint8_t    unkD { 0 };
		std::uint8_t    unkE { 0 };
		std::uint8_t    currentVariation { 0 };
		std::uint16_t   ptrParts { 0 };
		std::uint16_t   materialIdx { 0 };
		std::uint32_t   totalVariations { 0 };
		std::uint16_t   ptrObjects { 0 };
		std::uint16_t   ptrObjects_HI { 0 };
		std::uint32_t   unk3 { 0 };
		BoundingBox     boundingBox {};

		static void deserialize(PRMDescriptionChunkBaseHeader& header, ZBio::ZBinaryReader::BinaryReader *binaryReader);
	};
}