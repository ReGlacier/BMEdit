#pragma once

#include <cstdint>


namespace gamelib::prm
{
	enum class PRMVertexBufferFormat : std::uint8_t
	{
		VBF_VERTEX_10 = 0x10,
		VBF_VERTEX_24 = 0x24,
		VBF_VERTEX_28 = 0x28,
		VBF_VERTEX_34 = 0x34,

		VBF_SIMPLE_VERTEX = VBF_VERTEX_10,
		VBF_UNKNOWN_VERTEX = 0x0
	};
}