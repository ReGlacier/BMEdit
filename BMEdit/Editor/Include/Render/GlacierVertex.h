#pragma once

#include <Render/VertexFormatDescription.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>


namespace render
{
	struct GlacierVertex
	{
		glm::vec3 vPos {};
		glm::vec2 vUV {};

		static const VertexFormatDescription g_FormatDescription;
	};
}