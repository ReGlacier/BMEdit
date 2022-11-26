#pragma once

#include <GameLib/PRM/PRMVertexFormat.h>


namespace gamelib::prm
{
	struct PRMVertexBufferHeader
	{
		PRMVertexBufferFormat vertexFormat { PRMVertexBufferFormat::VBF_UNKNOWN_VERTEX };
	};
}