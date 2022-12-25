#include <BMRender/Mesh.h>
#include <glad/glad.h>
#include <algorithm>
#include <BMRender/RenderGL.h>
#include <BMRender/BufferDescription.h>


namespace bmr
{
	namespace impl
	{
		GLenum descriptionFieldTypeToGLType(DescriptionFieldType type)
		{
			switch (type)
			{
				case DescriptionFieldType::DFT_Unknown: return GL_NONE;
				case DescriptionFieldType::DFT_Bool: return GL_BOOL;
				case DescriptionFieldType::DFT_Int: return GL_INT;
				case DescriptionFieldType::DFT_UInt: return GL_UNSIGNED_INT;
				case DescriptionFieldType::DFT_Float: return GL_FLOAT;
				case DescriptionFieldType::DFT_Vec2: return GL_FLOAT;
				case DescriptionFieldType::DFT_Vec3: return GL_FLOAT;
				case DescriptionFieldType::DFT_Vec4: return GL_FLOAT;
				case DescriptionFieldType::DFT_IVec2: return GL_INT;
				case DescriptionFieldType::DFT_IVec3: return GL_INT;
				case DescriptionFieldType::DFT_IVec4: return GL_INT;
				case DescriptionFieldType::DFT_Mat3x3: return GL_FLOAT;
				case DescriptionFieldType::DFT_Mat4x4: return GL_FLOAT;
			}

			return GL_NONE;
		}

		GLenum meshTopologyToGLTopology(MeshTopology topology)
		{
			switch (topology) {
				case MeshTopology::MT_TRIANGLES: return GL_TRIANGLES;
				case MeshTopology::MT_TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
				case MeshTopology::MT_TRIANGLE_FAN: return GL_TRIANGLE_FAN;
				case MeshTopology::MT_LINES: return GL_LINES;
				case MeshTopology::MT_LINE_STRIP: return GL_LINE_STRIP;
			}

			return GL_NONE;
		}

		void declareVertexAttribute(int index, DescriptionFieldType type, size_t size, bool isNormalized, int offset, int stride)
		{
			const int componentsNr = static_cast<int>(size) / 4;
			GLboolean normalized = isNormalized ? GL_TRUE : GL_FALSE;
			GLenum glType = descriptionFieldTypeToGLType(type);

			glEnableVertexAttribArray(index); BM_RENDER_GL_CHECK_STATE()
			glVertexAttribPointer(index, componentsNr, glType, normalized, stride, reinterpret_cast<const void *>(offset)); BM_RENDER_GL_CHECK_STATE()
		}

		void setupBufferDescription(MeshFormat format)
		{
			static const BufferDescription mf10 = BufferDescription()
			    .addField(0, DescriptionFieldType::DFT_Vec3)
			    .addField(1, DescriptionFieldType::DFT_Int);

			static const BufferDescription mf24 = BufferDescription()
				.addField(0, DescriptionFieldType::DFT_Vec3)
			    .addField(1, DescriptionFieldType::DFT_Vec2)
			    .addField(2, DescriptionFieldType::DFT_IVec2)
			    .addField(3, DescriptionFieldType::DFT_Vec2);

			static const BufferDescription mf28 = BufferDescription()
				.addField(0, DescriptionFieldType::DFT_Vec3)
			    .addField(1, DescriptionFieldType::DFT_IVec2)
			    .addField(2, DescriptionFieldType::DFT_Vec2)
			    .addField(3, DescriptionFieldType::DFT_IVec3);

			static const BufferDescription mf34 = BufferDescription()
				.addField(0, DescriptionFieldType::DFT_Vec3)
			    .addField(1, DescriptionFieldType::DFT_Vec3)
			    .addField(2, DescriptionFieldType::DFT_IVec3)
			    .addField(3, DescriptionFieldType::DFT_Vec2)
			    .addField(4, DescriptionFieldType::DFT_IVec2);

			switch (format)
			{
				case MeshFormat::MF_UNKNOWN:
					return;

				case MeshFormat::MF_FORMAT_10:
					mf10.forEach(declareVertexAttribute);
					break;

				case MeshFormat::MF_FORMAT_24:
				    mf24.forEach(declareVertexAttribute);
					break;

				case MeshFormat::MF_FORMAT_28:
				    mf28.forEach(declareVertexAttribute);
					break;

				case MeshFormat::MF_FORMAT_34:
					mf34.forEach(declareVertexAttribute);
					break;
			}
		}

		SimpleMeshData::SimpleMeshData() = default;

		SimpleMeshData::~SimpleMeshData()
		{
			if (vao != kInvalidIdx)
			{
				glDeleteVertexArrays(1, &vao); BM_RENDER_GL_CHECK_STATE()
			}

			if (vbo != kInvalidIdx)
			{
				glDeleteBuffers(1, &vbo); BM_RENDER_GL_CHECK_STATE()
			}

			vao = kInvalidIdx;
			vbo = kInvalidIdx;
		}

		void SimpleMeshData::setup(MeshFormat format, BufferView vertices, int verticesCount)
		{
			if (vao != kInvalidIdx) glDeleteVertexArrays(1, &vao); BM_RENDER_GL_CHECK_STATE()
			if (vbo != kInvalidIdx) glDeleteBuffers(1, &vbo);      BM_RENDER_GL_CHECK_STATE()

			// VAO (attachments)
			glGenVertexArrays(1, reinterpret_cast<GLuint*>(&vao));      BM_RENDER_GL_CHECK_STATE()
			glBindVertexArray(vao);                                     BM_RENDER_GL_CHECK_STATE()

			// VBO (upload)
			glGenBuffers(1, reinterpret_cast<GLuint*>(&vbo));           BM_RENDER_GL_CHECK_STATE()
			glBindBuffer(GL_ARRAY_BUFFER, vbo);                         BM_RENDER_GL_CHECK_STATE()
			glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size), vertices.data, GL_STATIC_DRAW);    BM_RENDER_GL_CHECK_STATE()

			// VAO (description)
			setupBufferDescription(format);

			// Unbind all
			glBindBuffer(GL_ARRAY_BUFFER, 0);                                               BM_RENDER_GL_CHECK_STATE()
			glBindVertexArray(0);                                                           BM_RENDER_GL_CHECK_STATE()

			verticesNr = verticesCount;
		}

		void SimpleMeshData::enable()
		{
			if (vao != kInvalidIdx)
			{
				glBindVertexArray(vao); BM_RENDER_GL_CHECK_STATE()
			}
		}

		void SimpleMeshData::disable()
		{
			glBindVertexArray(0); BM_RENDER_GL_CHECK_STATE()
		}

		void SimpleMeshData::draw(MeshTopology topology)
		{
			if (vao != kInvalidIdx)
			{
				glDrawArrays(meshTopologyToGLTopology(topology), 0, verticesNr); BM_RENDER_GL_CHECK_STATE()
			}
		}

		/// ********************************************************************************
		IndexedMeshData::IndexedMeshData() = default;

		IndexedMeshData::~IndexedMeshData()
		{
			if (vao != kInvalidIdx)
			{
				glDeleteVertexArrays(1, &vao);   BM_RENDER_GL_CHECK_STATE()
			}

			if (vbo != kInvalidIdx && ebo != kInvalidIdx)
			{
				glDeleteBuffers(1, &vbo);        BM_RENDER_GL_CHECK_STATE()
				glDeleteBuffers(1, &ebo);        BM_RENDER_GL_CHECK_STATE()
			}

			vao = kInvalidIdx;
			vbo = kInvalidIdx;
			ebo = kInvalidIdx;
		}

		void IndexedMeshData::setup(MeshFormat format, BufferView vertices, BufferView indices, int indicesCount, int verticesCount)
		{
			if (vao != kInvalidIdx) glDeleteVertexArrays(1, &vao);
			if (vbo != kInvalidIdx) glDeleteBuffers(1, &vbo);
			if (ebo != kInvalidIdx) glDeleteBuffers(1, &ebo);

			// VAO
			glGenVertexArrays(1, &vao);  BM_RENDER_GL_CHECK_STATE()
			glBindVertexArray(vao); BM_RENDER_GL_CHECK_STATE()

			// VBO (upload)
			glGenBuffers(1, &vbo);       BM_RENDER_GL_CHECK_STATE()
			glBindBuffer(GL_ARRAY_BUFFER, vbo); BM_RENDER_GL_CHECK_STATE()
			glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size), vertices.data, GL_STATIC_DRAW); BM_RENDER_GL_CHECK_STATE()

			// EBO
			glGenBuffers(1, &ebo);       BM_RENDER_GL_CHECK_STATE()
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); BM_RENDER_GL_CHECK_STATE()
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size), indices.data, GL_STATIC_DRAW); BM_RENDER_GL_CHECK_STATE()

			// VAO (description)
			setupBufferDescription(format);

			// Unbind all
			glBindBuffer(GL_ARRAY_BUFFER, 0); BM_RENDER_GL_CHECK_STATE()
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); BM_RENDER_GL_CHECK_STATE()
			glBindVertexArray(0); BM_RENDER_GL_CHECK_STATE()

			indicesNr = indicesCount;
			verticesNr = verticesCount;
		}

		void IndexedMeshData::enable()
		{
			if (vao != kInvalidIdx && ebo != kInvalidIdx)
			{
				glBindVertexArray(vao); BM_RENDER_GL_CHECK_STATE()
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); BM_RENDER_GL_CHECK_STATE()
			}
		}

		void IndexedMeshData::disable()
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); BM_RENDER_GL_CHECK_STATE()
			glBindVertexArray(0); BM_RENDER_GL_CHECK_STATE()
		}

		void IndexedMeshData::draw(MeshTopology topology)
		{
			if (vao != kInvalidIdx && ebo != kInvalidIdx)
			{
				glDrawElements(meshTopologyToGLTopology(topology), indicesNr, GL_UNSIGNED_SHORT, nullptr); BM_RENDER_GL_CHECK_STATE()
			}
		}
	}

	Mesh::Mesh() = default;

	void Mesh::setMeshData(MeshFormat format, BufferView vertexBuffer, int verticesCount)
	{
		auto& md = m_data.emplace<impl::SimpleMeshData>();
		md.setup(format, vertexBuffer, verticesCount);
		m_hasData = true;
	}

	void Mesh::setMeshData(MeshFormat format, BufferView vertexBuffer, BufferView indexBuffer, int indicesCount, int verticesCount)
	{
		auto& md = m_data.emplace<impl::IndexedMeshData>();
		md.setup(format, vertexBuffer, indexBuffer, indicesCount, verticesCount);
		m_hasData = true;
	}

	void Mesh::setTopology(MeshTopology topology)
	{
		m_topology = topology;
	}

	MeshTopology Mesh::getTopology() const
	{
		return m_topology;
	}

	void Mesh::draw()
	{
		std::visit([&](auto&& md) {
			md.enable();
			md.draw(m_topology);
			md.disable();
		}, m_data);
	}
}