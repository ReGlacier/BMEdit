#include <Render/GlacierVertex.h>
#include <Render/Model.h>


namespace render
{
	static constexpr GLenum g_entryTypeToGLType[] = {
	    GL_NONE,
	    GL_BOOL,
	    GL_INT,
	    GL_UNSIGNED_INT,
	    GL_FLOAT,
	    GL_FLOAT,
	    GL_FLOAT,
	    GL_FLOAT,
	    GL_INT,
	    GL_INT,
	    GL_INT,
	    GL_FLOAT,
	    GL_FLOAT,
	};

	bool Mesh::setup(QOpenGLFunctions_3_3_Core* gapi, const VertexFormatDescription& vertexFormat, const uint8_t* vertices, uint32_t verticesCount, const uint8_t* indices, uint32_t indicesCount, bool bDynamic)
	{
		// Allocate resources
		gapi->glGenVertexArrays(1, &vao);
		gapi->glGenBuffers(1, &vbo);
		if (indices != nullptr)
			gapi->glGenBuffers(1, &ibo);

		// Attach VAO
		gapi->glBindVertexArray(vao);
		gapi->glBindBuffer(GL_ARRAY_BUFFER, vbo);

		// Upload vertices
		gapi->glBufferData(GL_ARRAY_BUFFER, vertexFormat.getStride() * verticesCount, vertices, bDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

		// Upload indices (if required)
		if (indices != nullptr)
		{
			gapi->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			gapi->glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(uint16_t) * indicesCount), indices, bDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
		}

		// Initialize vertex format
		vertexFormat.visit([gapi](uint32_t index, uint32_t offset, uint32_t size, uint32_t stride, VertexDescriptionEntryType t, bool isNormalized) {
			const int componentsNr = static_cast<int>(size) / 4;
			const GLboolean normalized = isNormalized ? GL_TRUE : GL_FALSE;
			const GLenum glType = g_entryTypeToGLType[static_cast<int>(t)];

			gapi->glEnableVertexAttribArray(index);
			gapi->glVertexAttribPointer(index, componentsNr, glType, normalized, static_cast<GLsizei>(stride), reinterpret_cast<const void*>(static_cast<std::ptrdiff_t>(offset)));
		});

		// Save data
		m_vertexFormat = vertexFormat;
		m_bIsDynamic = bDynamic;

		if (indices != nullptr)
		{
			trianglesCount = static_cast<int>(indicesCount / 3);
		}
		else
		{
			trianglesCount = static_cast<int>(verticesCount / 3);
		}

		m_maxVerticesNr = verticesCount;
		m_maxIndicesNr = indices != nullptr ? indicesCount : 0u;

		return true;
	}

	bool Mesh::update(QOpenGLFunctions_3_3_Core* gapi, uint32_t verticesOffset, const uint8_t* vertices, uint32_t verticesCount, uint32_t indicesOffset, const uint8_t* indices, uint32_t indicesCount) // NOLINT(*-make-member-function-const)
	{
		gapi->glBindBuffer(GL_ARRAY_BUFFER, vbo);
		gapi->glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLsizei>(verticesOffset), static_cast<GLsizeiptr>(verticesCount * m_vertexFormat.getStride()), vertices);
		gapi->glBindBuffer(GL_ARRAY_BUFFER, 0);

		if (indices)
		{
			gapi->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			gapi->glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizei>(indicesOffset), static_cast<GLsizeiptr>(indicesCount * sizeof(uint16_t)), indices);
			gapi->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		return true;
	}

	void Mesh::discard(QOpenGLFunctions_3_3_Core *gapi)
	{
		if (vao != kInvalidResource)
		{
			gapi->glDeleteVertexArrays(1, &vao);
			vao = kInvalidResource;
		}

		if (vbo != kInvalidResource)
		{
			gapi->glDeleteBuffers(1, &vbo);
			vbo = kInvalidResource;
		}

		if (ibo != kInvalidResource)
		{
			gapi->glDeleteBuffers(1, &ibo);
			ibo = kInvalidResource;
		}

		m_vertexFormat = {};
		m_maxVerticesNr = 0u;
		m_maxIndicesNr = 0u;
		m_bIsDynamic = false;
		trianglesCount = 0;
	}

	void Model::discard(QOpenGLFunctions_3_3_Core *gapi)
	{
		if (boundingBoxMesh.has_value())
		{
			boundingBoxMesh.value().discard(gapi);
			boundingBoxMesh = std::nullopt;
		}

		for (auto& mesh : meshes)
		{
			mesh.discard(gapi);
		}

		meshes.clear();
	}

	bool Model::setupBoundingBox(QOpenGLFunctions_3_3_Core *gapi)
	{
		VertexFormatDescription vertexFormat{};
		vertexFormat.addField(0, VertexDescriptionEntryType::VDE_Vec3, false);

		const glm::vec3& vMin = boundingBox.min;
		const glm::vec3& vMax = boundingBox.max;

		std::vector<SimpleVertex> vertices {
		    glm::vec3{vMin.x, vMin.y, vMin.z}, glm::vec3{vMin.x, vMin.y, vMax.z},
		    glm::vec3{vMin.x, vMax.y, vMin.z}, glm::vec3{vMin.x, vMax.y, vMax.z},
		    glm::vec3{vMax.x, vMin.y, vMin.z}, glm::vec3{vMax.x, vMin.y, vMax.z},
		    glm::vec3{vMax.x, vMax.y, vMin.z}, glm::vec3{vMax.x, vMax.y, vMax.z}
		};

		std::vector<uint16_t> indices {
		    0, 1, 1, 3, 3, 2, 2, 0,
		    4, 5, 5, 7, 7, 6, 6, 4,
		    0, 4, 1, 5, 2, 6, 3, 7
		};

		Mesh& mesh = boundingBoxMesh.emplace();
		mesh.glTextureId = 0u;
		mesh.materialId = 0u;
		return mesh.setup(gapi, vertexFormat, vertices, indices, false);
	}

	void Mesh::render(QOpenGLFunctions_3_3_Core* gapi, render::RenderTopology topology) const
	{
		if (vao == kInvalidResource)
		{
			assert(false && "Not initialised!");
			return;
		}

		assert(trianglesCount > 0);

		// Select topology
		GLenum glTopology = GL_NONE;
		switch (topology)
		{
			case RenderTopology::RT_NONE:
			{
				assert(false && "Invalid topology");
				return;
		    }
			case RenderTopology::RT_POINTS: glTopology = GL_POINTS; break;
			case RenderTopology::RT_LINES: glTopology = GL_LINES; break;
			case RenderTopology::RT_LINE_STRIP: glTopology = GL_LINE_STRIP; break;
			case RenderTopology::RT_LINE_LOOP: glTopology = GL_LINE_LOOP; break;
			case RenderTopology::RT_TRIANGLES: glTopology = GL_TRIANGLES; break;
			case RenderTopology::RT_TRIANGLE_STRIP: glTopology = GL_TRIANGLE_STRIP; break;
			case RenderTopology::RT_TRIANGLE_FAN: glTopology = GL_TRIANGLE_FAN; break;
		}


		// Activate us
		gapi->glBindVertexArray(vao);

		// Perform draw
		if (ibo != kInvalidResource)
		{
			// Indexed
			gapi->glDrawElements(glTopology, (trianglesCount * 3), GL_UNSIGNED_SHORT, nullptr);
		}
		else
		{
			// Arrays
			gapi->glDrawArrays(GL_TRIANGLES, 0, trianglesCount);
		}

		// Deactivate us
		gapi->glBindVertexArray(0);
	}
}