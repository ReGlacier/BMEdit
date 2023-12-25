#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <Render/VertexFormatDescription.h>
#include <Render/RenderTopology.h>
#include <GameLib/BoundingBox.h>
#include <Render/GLResource.h>
#include <optional>
#include <cstdint>
#include <vector>


namespace render
{
	class Mesh
	{
	private:
		VertexFormatDescription m_vertexFormat {};
		uint32_t m_maxVerticesNr { 0u };
		uint32_t m_maxIndicesNr { 0u };
		bool m_bIsDynamic { false };

	public:
		GLuint vao { kInvalidResource };
		GLuint vbo { kInvalidResource };
		GLuint ibo { kInvalidResource };
		GLuint glTextureId { kInvalidResource }; /// Render OpenGL texture resource handle
		uint16_t materialId { 0 }; /// Id of material from Glacier mesh (just copy)
		uint8_t variationId { 0 }; // Id of variation (some meshes could be attached to abstract 'variation' so each variation could be interpreted as 'group of meshes')

		int trianglesCount { 0 };

		[[nodiscard]] bool isDynamic() const { return m_bIsDynamic; }

		/**
		 * @brief Construct and upload vertex data into VAO + VBO + IBO
		 * @note For dynamic buffers allowed to use upload(...) method. Otherwise it will do nothing
		 * @tparam TVertex
		 * @param gapi
		 * @param vertexFormat
		 * @param vertices
		 * @param indices
		 * @param bIsDynamic
		 * @return
		 */
		template <typename TVertex>
		bool setup(QOpenGLFunctions_3_3_Core* gapi, const VertexFormatDescription& vertexFormat, const std::vector<TVertex>& vertices, const std::vector<uint16_t>& indices, bool bIsDynamic)
		{
			if (vao != kInvalidResource || vbo != kInvalidResource || ibo != kInvalidResource)
			{
				assert(false && "Need discard resource before create a new one!");
				return false;
			}

			if (!gapi || vertexFormat.getEntries().empty() || vertices.empty())
				return false;

			return setup(gapi,
			             vertexFormat,
			             reinterpret_cast<const uint8_t*>(vertices.data()), static_cast<uint32_t>(vertices.size()),
			             indices.empty() ? nullptr : reinterpret_cast<const uint8_t*>(indices.data()), indices.empty() ? 0 : static_cast<uint32_t>(indices.size()),
			             bIsDynamic);
		}

		/**
		 * @brief Update vertex and index buffer (able to update only vertex buffer, but unable to update only index buffer because it may be a root of inconsistent)
		 * @note This method will return false when not enough space in buffer (initialized at setup)!
		 * @note This method will return false when user trying to upload index buffer without initialise index buffer in setup!
		 * @tparam TVertex
		 * @param gapi
		 * @param vertices
		 * @param indices
		 * @return
		 */
		template <typename TVertex>
		bool update(QOpenGLFunctions_3_3_Core* gapi, uint32_t verticesOffset, const std::vector<TVertex>& vertices, uint32_t indicesOffset = 0, const std::vector<uint16_t>& indices = {})
		{
			if (!gapi || vertices.empty())
				return false;

			if (!m_bIsDynamic)
				return false;

			if (vao == kInvalidResource || vbo == kInvalidResource)
			{
				assert(false && "Call setup() before update!");
				return false;
			}

			return update(gapi,
			              verticesOffset, reinterpret_cast<const uint8_t*>(vertices.data()), static_cast<uint32_t>(vertices.size()),
			              indicesOffset, indices.empty() ? nullptr : reinterpret_cast<const uint8_t*>(indices.data()), indices.empty() ? 0 : static_cast<uint32_t>(indices.size()));
		}

		/**
		 * @brief Discard all resources and makes object invalid
		 * @param gapi
		 */
		void discard(QOpenGLFunctions_3_3_Core* gapi);

		/**
		 * @brief Do render of mesh
		 * @param gapi
		 * @param topology - which element topology stored inside buffer
		 */
		void render(QOpenGLFunctions_3_3_Core* gapi, RenderTopology topology = RenderTopology::RT_TRIANGLES) const;

	private:
		/**
		 * @brief Create & upload vertices & indices into a single mesh
		 * @param gapi
		 * @param vertexFormat
		 * @param vertices
		 * @param verticesCount
		 * @param indices
		 * @param indicesCount
		 * @return true if everything is ok
		 */
		bool setup(QOpenGLFunctions_3_3_Core* gapi, const VertexFormatDescription& vertexFormat, const uint8_t* vertices, uint32_t verticesCount, const uint8_t* indices, uint32_t indicesCount, bool bDynamic);

		/**
		 * @brief Update vertex & index buffer (or only vertex buffer)
		 * @param gapi
		 * @param verticesOffset
		 * @param vertices
		 * @param verticesCount
		 * @param indices
		 * @param indicesCount
		 * @param indicesOffset
		 * @return
		 */
		bool update(QOpenGLFunctions_3_3_Core* gapi, uint32_t verticesOffset, const uint8_t* vertices, uint32_t verticesCount, uint32_t indicesOffset, const uint8_t* indices, uint32_t indicesCount);
	};

	struct Model
	{
		std::vector<Mesh> meshes {};
		std::optional<Mesh> boundingBoxMesh {};  // mesh with bounding box
		gamelib::BoundingBox boundingBox {};
		[[maybe_unused]] uint32_t chunkId {0u};

		void discard(QOpenGLFunctions_3_3_Core* gapi);

		bool setupBoundingBox(QOpenGLFunctions_3_3_Core* gapi);
	};
}