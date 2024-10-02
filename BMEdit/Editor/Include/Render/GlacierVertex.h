#pragma once

#include <Render/VertexFormatDescription.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>


namespace render
{
#pragma pack(push, 1)
	struct GlacierVertex
	{
		glm::vec3 vPos {};
		glm::vec2 vUV {};

		static const VertexFormatDescription g_FormatDescription;
	};
#pragma pack(pop)

	struct SimpleVertex
	{
		glm::vec3 vPos {};

		SimpleVertex() = default;
		SimpleVertex(const glm::vec3& v1) : vPos(v1) {}

		static const VertexFormatDescription g_FormatDescription;
	};
}