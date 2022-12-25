#pragma once

#include <cstdint>


namespace gamelib::prm
{
	enum class PRMVertexFormatFeature : std::uint8_t
	{
		VFF_POSITION = 1 << 0,
		VFF_UV = 1 << 1
	};
}