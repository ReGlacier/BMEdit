#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <array>

#include <type_traits>


namespace gamelib
{
	struct BoundingBox
	{
		glm::vec3 min;
		glm::vec3 max;

		BoundingBox() = default;
		BoundingBox(const glm::vec3 &vMin, const glm::vec3 &vMax);

		glm::vec3 getCenter() const;

		void expand(const BoundingBox& another);
		bool contains(const glm::vec3& vPoint) const;

		static BoundingBox toWorld(const BoundingBox& source, const glm::mat4& mTransform);

		template <typename TOutVertexIterator, typename TOutIndexIterator>
		void toLines(TOutVertexIterator outVertexIt, TOutIndexIterator outIndexIt)
		{
			// Compute vertices
			std::array<glm::vec3, 16> aVertices {
			    glm::vec3{min.x, min.y, min.z}, glm::vec3{min.x, min.y, max.z},
			    glm::vec3{min.x, max.y, min.z}, glm::vec3{min.x, max.y, max.z},
			    glm::vec3{max.x, min.y, min.z}, glm::vec3{max.x, min.y, max.z},
			    glm::vec3{max.x, max.y, min.z}, glm::vec3{max.x, max.y, max.z}
			};

			// Compute indices
			std::array<uint16_t, 32> aIndices {
			    0, 1, 1, 3, 3, 2, 2, 0,
			    4, 5, 5, 7, 7, 6, 6, 4,
			    0, 4, 1, 5, 2, 6, 3, 7
			};

			for (const auto& vVertex : aVertices)
			{
				using TP = typename TOutVertexIterator::container_type::value_type;
				TP vProxy;
				vProxy.vPos = vVertex;
				(*outVertexIt++) = std::move(vProxy);
			}

			for (const auto& iIndex : aIndices)
			{
				(*outIndexIt++) = iIndex;
			}
		}
	};
}