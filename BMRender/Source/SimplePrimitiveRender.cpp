#include <glad/glad.h>
#include <BMRender/SimplePrimitiveRender.h>
#include <glm/gtc/matrix_transform.hpp>


namespace bmr
{
	namespace consts
	{
		// Material
		constexpr const char *kColor = "i_Color";
		constexpr const char *kProjView = "i_ProjView";
		constexpr const char *kModel = "i_Model";

		// Format
		constexpr const char *kGlacier_Format10 = "GLACIER_USE_FORMAT_10";
		constexpr const char *kGlacier_Format24 = "GLACIER_USE_FORMAT_24";
		constexpr const char *kGlacier_Format28 = "GLACIER_USE_FORMAT_28";
		constexpr const char *kGlacier_Format34 = "GLACIER_USE_FORMAT_34";
	}

	SimplePrimitiveRender::SimplePrimitiveRender(const gamelib::Level *level)
		: m_level(level)
	{
	}

	void SimplePrimitiveRender::setPrimitiveIndex(std::uint32_t primId)
	{
		if (m_primitiveIndex != primId)
		{
			m_primitiveChanged = true;
			m_primitiveIndex = primId;
		}
	}

	void SimplePrimitiveRender::resetPrimitiveIndex()
	{
		if (m_primitiveIndex != 0u)
		{
			m_primitiveIndex = 0u;
			m_primitiveChanged = true;
		}
	}

	std::uint32_t SimplePrimitiveRender::getPrimitiveIndex() const
	{
		return m_primitiveIndex;
	}

	void SimplePrimitiveRender::setWireframeRenderEnabled(bool isEnabled)
	{
	}

	void SimplePrimitiveRender::setBoundingBoxRenderingEnabled(bool isEnabled)
	{
	}

	Camera &SimplePrimitiveRender::getCamera()
	{
		return m_camera;
	}

	const Camera &SimplePrimitiveRender::getCamera() const
	{
		return m_camera;
	}

	bool SimplePrimitiveRender::isWireframeRenderingEnabled() const
	{
		return false;
	}

	bool SimplePrimitiveRender::isBoundingBoxRenderingEnabled() const
	{
		return false;
	}

	void SimplePrimitiveRender::onSetupFinished()
	{
		m_primaryMaterial = std::make_unique<ShaderProgram>();
		m_primaryMaterial->addDefinition(consts::kGlacier_Format10);

		auto mtlCompileResult = m_primaryMaterial->compile({
		    {
		        ShaderSourceCodeType::SRC_FRAGMENT,
		        R"(
#version 330 core

// Input
in vec2 g_TexCoord;  /// Unused for now

// Result
out vec4 o_FragColor;

// Uniforms
uniform vec4 i_Color;

void main()
{
    o_FragColor = i_Color;
}
)"
		    },
		    {
		        ShaderSourceCodeType::SRC_VERTEX,
		        R"(
#version 330 core

// Attributes
#if defined(GLACIER_USE_FORMAT_10)
layout(location = 0) in vec3 aPos; // X,Y,Z position
layout(location = 1) in int aUnk1; // Some unknown data, maybe color ?
#elif defined(GLACIER_USE_FORMAT_24)
layout(location = 0) in vec3  aPos; // X,Y,Z position
layout(location = 1) in vec2  aUV;  // U, V position
layout(location = 2) in ivec2 unk1; // Unknown data (int[2])
layout(location = 3) in vec2  aUV2; // U1, V1 position
#elif defined(GLACIER_USE_FORMAT_28)
layout(location = 0) in vec3  aPos; // X,Y,Z position
layout(location = 1) in ivec2 unk1; // Unknown data (int[2])
layout(location = 2) in vec2  aUV;  // U, V position
layout(location = 3) in ivec3 unk2; // Unknown data (int[3])
#elif defined(GLACIER_USE_FORMAT_34)
layout(location = 0) in vec3  aPos; // X,Y,Z position
layout(location = 1) in vec3  unk1; // Unknown data (float[3])
layout(location = 2) in ivec3 unk2; // Unknown data (int[3])
layout(location = 3) in vec2  aUV;  // U, V
layout(location = 4) in ivec2 unk3; // Unknown data (int[2])
#else
#error "UNKNOWN VERTEX FORMAT!"
#endif

// MVP
uniform mat4 i_Model;
uniform mat4 i_ProjView;

// Output
out vec2 g_TexCoord;

void main() {
    gl_Position = i_ProjView * i_Model * vec4(aPos, 1.0);
    g_TexCoord = vec2(0.0);
}
)"
		    }
		});

		if (!mtlCompileResult)
		{
			printf("MATERIAL ERROR: %s\n", mtlCompileResult.errorMessage.data());
			return;
		}

		m_mesh = std::make_unique<Mesh>();
		m_boundingBox = std::make_unique<Mesh>();

		const auto& opt = getOptions();

		m_camera = Camera { opt.fov, opt.width, opt.height };
	}

	void SimplePrimitiveRender::onSizeChangedImpl(int width, int height)
	{
		m_camera.setCameraScreenSize(width, height);
	}

	void SimplePrimitiveRender::onBeginFrame()
	{
		if (m_primitiveChanged)
		{
			setupPrimitive();
		}

		// Select & bind shader
		if (!m_primaryMaterial || !m_mesh)
		{
			return;
		}

		// Apply material
		{
			m_primaryMaterial->enable();
			m_primaryMaterial->setParameter(consts::kModel, m_meshModelMatrix);
			m_primaryMaterial->setParameter(consts::kProjView, m_camera.getProjViewMatrix());
		}

		// Draw bounding box
		{
			m_primaryMaterial->setParameter(consts::kColor, glm::vec4(0.f, 0.0f, 1.0f, 1.f));
			m_boundingBox->draw();
		}

		// Draw primitive
		{
			m_primaryMaterial->setParameter(consts::kColor, glm::vec4(.0f, 1.0f, .0f, 1.f));
			m_mesh->draw();
		}
	}

	void SimplePrimitiveRender::onEndFrame()
	{
		if (!m_primaryMaterial || !m_mesh)
		{
			return;
		}

		// Material
		m_primaryMaterial->disable();
	}

	void SimplePrimitiveRender::setupPrimitive()
	{
		m_primitiveChanged = false;

		if (!m_primitiveIndex)
		{
			return;
		}

		// Extract mesh format
		auto &selfChunk   = const_cast<gamelib::Level*>(m_level)->getLevelGeometry()->chunks.at(m_primitiveIndex + 0);
		auto &indexChunk  = const_cast<gamelib::Level*>(m_level)->getLevelGeometry()->chunks.at(m_primitiveIndex + 1);
		auto &vertexChunk = const_cast<gamelib::Level*>(m_level)->getLevelGeometry()->chunks.at(m_primitiveIndex + 2);

		if (selfChunk.getKind() != gamelib::prm::PRMChunkRecognizedKind::CRK_DESCRIPTION_BUFFER ||
		    indexChunk.getKind() != gamelib::prm::PRMChunkRecognizedKind::CRK_INDEX_BUFFER ||
		    vertexChunk.getKind() != gamelib::prm::PRMChunkRecognizedKind::CRK_VERTEX_BUFFER)
		{
			m_primitiveIndex = 0u; // switch to invalid index
			return;
		}

		// Primitive mesh
		{
			std::string vertexFormatDef;
			MeshFormat meshFormat;

			switch (vertexChunk.getVertexBufferHeader()->vertexFormat)
			{
			case gamelib::prm::PRMVertexBufferFormat::VBF_VERTEX_10:
				vertexFormatDef = consts::kGlacier_Format10;
				meshFormat = MeshFormat::MF_FORMAT_10;
				break;
			case gamelib::prm::PRMVertexBufferFormat::VBF_VERTEX_24:
				vertexFormatDef = consts::kGlacier_Format24;
				meshFormat = MeshFormat::MF_FORMAT_24;
				break;
			case gamelib::prm::PRMVertexBufferFormat::VBF_VERTEX_28:
				vertexFormatDef = consts::kGlacier_Format28;
				meshFormat = MeshFormat::MF_FORMAT_28;
				break;
			case gamelib::prm::PRMVertexBufferFormat::VBF_VERTEX_34:
				vertexFormatDef = consts::kGlacier_Format34;
				meshFormat = MeshFormat::MF_FORMAT_34;
				break;
			case gamelib::prm::PRMVertexBufferFormat::VBF_UNKNOWN_VERTEX:
				return;
			}

			m_primaryMaterial->removeDefinition(consts::kGlacier_Format10);
			m_primaryMaterial->removeDefinition(consts::kGlacier_Format24);
			m_primaryMaterial->removeDefinition(consts::kGlacier_Format28);
			m_primaryMaterial->removeDefinition(consts::kGlacier_Format34);
			m_primaryMaterial->addDefinition(vertexFormatDef);

			BufferView vertexBuffer{vertexChunk.getBuffer().data(), vertexChunk.getBufferSize()};
			BufferView indexBuffer{indexChunk.getBuffer().data(), indexChunk.getBufferSize()};

			const int verticesCount = static_cast<int>(vertexBuffer.size) / static_cast<int>(meshFormat);

			std::uint16_t unk1{0};
			std::uint16_t indexCount{0};

			indexBuffer >> unk1;
			indexBuffer >> indexCount;

			m_mesh->setMeshData(meshFormat, vertexBuffer, indexBuffer, indexCount, verticesCount);
		}

		// Bounding box mesh
		{
			struct V10
			{
				glm::vec3 xyz { .0f };
				std::uint32_t unk { 0u };

				V10() = default;
				V10(float x, float y, float z) : xyz(x, y, z), unk(0u) {}
			};

			std::array<V10, 8> aVertices {};
			std::array<std::uint16_t, 24> aIndices { 0 };
			selfChunk.getDescriptionBufferHeader()->boundingBox.getCubeAsLines(aVertices, aIndices);

			BufferView vertexBuffer { &aVertices[0], sizeof(V10) * aVertices.size() };
			BufferView indexBuffer { &aIndices[0], sizeof(std::uint16_t) * aIndices.size() };

			m_boundingBox->setMeshData(MeshFormat::MF_FORMAT_10, vertexBuffer, indexBuffer, aIndices.size(), aVertices.size());
			m_boundingBox->setTopology(MeshTopology::MT_LINES);
		}
	}
}