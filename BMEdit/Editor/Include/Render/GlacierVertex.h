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

	struct SimpleVertex
	{
		glm::vec3 vPos {};

		SimpleVertex();
		SimpleVertex(const glm::vec3& v1) : vPos(v1) {}

		static const VertexFormatDescription g_FormatDescription;
	};
}