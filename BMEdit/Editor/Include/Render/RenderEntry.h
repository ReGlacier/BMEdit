#pragma once

#include <cstdint>
#include <string>
#include <list>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#include <GameLib/MAT/MATRenderState.h>
#include <Render/RenderTopology.h>
#include <Render/Shader.h>
#include <Render/Model.h>


namespace render
{
	enum TextureSlotId : uint32_t
	{
		kMapDiffuse = 0,
		kMapSpecularMask,
		kMapEnvironment,
		kMapReflectionMask,
		kMapReflectionFallOff,
		kMapIllumination,
		kMapTranslucency,

		// should be last
		kMaxTextureSlot
	};

	struct RenderEntry
	{
		// Render params
		uint32_t iPrimitiveId { 0 }; // ID of primitive from PRM
		uint32_t iMeshIndex { 0 };   // Index of mesh from PRM primitive
		uint32_t iTrianglesNr { 0 }; // Count of triangles or elements
		RenderTopology renderTopology { RenderTopology::RT_TRIANGLES }; // Topology of geometry buffer

		// World params
		glm::vec3 vPosition { .0f }; // World position
		glm::mat4 mWorldTransform { 1.f }; // Converted and translated matrix (ready to use in OpenGL)
		glm::mat3 mLocalOriginalTransform { 1.f }; // Original local matrix

		// Mesh instance
		render::Mesh* pMesh { nullptr };

		struct Material
		{
			uint16_t id { 0 }; // Id of material from MAT file

			std::string sBaseMatClass {}; // Name of base material class
			std::string sInstanceMatName {}; // Name of material instance

			// Parameters
			glm::vec4 vDiffuseColor { 1.f };
			glm::vec4 gm_vZBiasOffset { .0f };
			glm::vec4 v4Opacity { 1.f };
			glm::vec4 v4Bias { .0f };
			int32_t iAlphaREF { 255 };

			// Render State
			gamelib::mat::MATRenderState renderState {};

			// Textures
			std::array<GLuint, TextureSlotId::kMaxTextureSlot> textures { 0 };

			// Shader program
			render::Shader* pShader { nullptr };
		} material;
	};

	using RenderEntriesList = std::list<RenderEntry>;
}