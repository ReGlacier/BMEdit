#pragma once

#include <cstdint>


namespace gamelib::tex
{
	/**
	 * @enum TEXEntryType
	 * @brief Describes all supported by Glacier texture formats
	 * @note BMEdit supports I8, U8V8, DXT1, DXT3, RGBA (32) and PAL formats. PAL_OPAC not used by Glacier but supported in missions (not in Loader_Sequence!).
	 *       EMBM, DOT3, CUBE and DMAP referred, supported (in theory) but we can operate it only via DirectX9 SDK, so not our case.
	 */
	enum class TEXEntryType : uint32_t
	{
		ET_BITMAP_I8       = 0x49382020u, //I8
		ET_BITMAP_EMBM     = 0x454D424Du, //EMBM
		ET_BITMAP_DOT3     = 0x444F5433u, //DOT3
		ET_BITMAP_CUBE     = 0x43554245u, //CUBE
		ET_BITMAP_DMAP     = 0x444D4150u, //DMAP
		ET_BITMAP_PAL      = 0x50414C4Eu, //PALN
		ET_BITMAP_PAL_OPAC = 0x50414C4Fu, //PALO
		ET_BITMAP_32       = 0x52474241u, //RGBA
		ET_BITMAP_U8V8     = 0x55385638u, //V8U8
		ET_BITMAP_DXT1     = 0x44585431u, //DXT1
		ET_BITMAP_DXT3     = 0x44585433u, //DXT3
	};
}