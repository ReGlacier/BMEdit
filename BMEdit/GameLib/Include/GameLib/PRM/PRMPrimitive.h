#pragma once

#include <cstdint>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::prm
{
	struct PRMPrimitive
	{
		uint8_t iBonesDeclOffset; // +0x0 (see ZPrimControlWintel::GetPrimInfoString for details)
		uint8_t iPrimPackType; // +0x1
		uint16_t iKind; //+0x2
		uint16_t iTextureId; // +0x4
		uint16_t unk6; // +0x6
		uint32_t iNextVariationOffset; // +0x8
		uint8_t unkC; // +0xC (see ZRenderObjectInstanceOldMeshD3D::sub_498B70 for details)
		bool unkD; // +0xD
		bool unkE; // +0xE
		uint8_t iCurrentVariationIndex; // +0xF
		uint16_t unk10; // +0x10
		uint16_t iMaterialIndex; // +0x12
		uint32_t iTotalVariations; // +0x14

		/**
		 * About this structure:
		 *    I have a thery about that this structure is a just base of whole structure. And the true size of structure defined by eKind field.
		 *    I know, that we have those possible values (see function at 0046CAB0):
		 *    	0  -
		 *    	1  - Unknown, but total structure size is 0xA0 (?). Theory: it's primitive with bones. Fn at 0046D680
		 *    	4  - (from function 0x00487480 & function 0x00487450)
		 *    	6  -
		 *    	7  - Generic model?
		 *    	8  - (see fn 00487740)
		 *    	10 -
		 *    	11 -
		 *    	12 - Fn at 0046D250, 0046D220, 0046D1F0, 0046D1C0, 0046CFE0 (!!!), 0046CFA0, 0046E1B0 (!!)
		 *
		 * And this structure contains information about lighting. There are 4 types:
		 * 		LightOmni (size 0x2C)
		 * 		LightSpot (size 0x40)
		 * 		LightSpotSquare (size 0x44)
		 * 		LightEnvironment (size 0x28)
		 */

		static void deserialize(PRMPrimitive &primitive, ZBio::ZBinaryReader::BinaryReader *binaryReader);
	};
}