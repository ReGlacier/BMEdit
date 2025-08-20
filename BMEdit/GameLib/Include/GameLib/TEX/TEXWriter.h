#pragma once

#include <GameLib/TEX/TEXHeader.h>
#include <GameLib/TEX/TEXTypes.h>
#include <GameLib/TEX/TEXEntry.h>
#include <cstdint>
#include <vector>
#include <array>


namespace gamelib::tex
{
	struct TEXWriter
	{
		static void write(const TEXHeader &header, const std::vector<TEXEntry> &entries, std::vector<uint8_t> &outBuffer);
	};
}