#pragma once

#include <GameLib/BoundingBox.h>
#include <GameLib/Vector3.h>
#include <cstdint>


namespace gamelib::prm
{
	struct PRMDescriptionChunkBaseHeader
	{
		std::uint8_t    boneDeclOffset;
		std::uint8_t    primPackType;
		std::uint16_t   kind;
		std::uint16_t   textureId;
		std::uint16_t   unk6;
		std::uint32_t   nextVariation;
		std::uint8_t    unkC;
		std::uint8_t    unkD;
		std::uint8_t    unkE;
		std::uint8_t    currentVariation;
		std::uint16_t   ptrParts;
		std::uint16_t   materialIdx;
		std::uint32_t   totalVariations;
		std::uint32_t   ptrObjects;
		std::uint32_t   unk3;
		BoundingBox     bounding_box;
	};
}