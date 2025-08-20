#pragma once

#include <cstdint>


namespace render
{
	enum class RenderTopology : uint8_t
	{
		RT_NONE = 0,
		RT_POINTS,
		RT_LINES,
		RT_LINE_STRIP,
		RT_LINE_LOOP,
		RT_TRIANGLES,
		RT_TRIANGLE_STRIP,
		RT_TRIANGLE_FAN
	};
}