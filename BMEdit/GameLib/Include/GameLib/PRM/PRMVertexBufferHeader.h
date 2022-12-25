#pragma once

#include <GameLib/PRM/PRMVertexFormat.h>
#include <cstddef>


namespace gamelib::prm
{
	struct PRMVertexBufferHeader
	{
		PRMVertexBufferFormat vertexFormat { PRMVertexBufferFormat::VBF_UNKNOWN_VERTEX };

		[[nodiscard]] std::size_t getVertexSize() const { return static_cast<std::size_t>(vertexFormat); }
	};
}