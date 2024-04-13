#pragma once

#include <GameLib/BoundingBox.h>
#include <glm/vec3.hpp>
#include <type_traits>


namespace gamelib
{
	template <typename T>
	concept THasPosition = requires(T t)
	{
		{ t.vPos };
	};

	class Plane
	{
	public:
		Plane();
		Plane(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);

		/**
		 * @brief Calculates bounding box which cover plane
		 */
		BoundingBox makeBoundingBox() const;

		/**
		 * @brief Calculates normal vector of plane
		 * @note This method using easy calculation method, so it can return wrong result for hard planes
		 */
		glm::vec3 getNormal() const;

		/**
		 * @return -getNormal
		 * @note See comment to getNormal for details
		 */
		glm::vec3 getBiNormal() const;

		/**
		 * @return center of plane (point, not a vector)
		 */
		glm::vec3 getCenter() const;

		/**
		 * @return Size of plane as maximum distance between points
		 */
		float getSize() const;

		template <typename TOutVertexIterator, typename TOutIndexIterator>
		void toTriangles(TOutVertexIterator outVertexIt, TOutIndexIterator outIndexIt) requires (THasPosition<typename TOutVertexIterator::container_type::value_type>)
		{
			// Push vertices
			for (const auto& v : m_aVertices)
			{
				using TP = typename TOutVertexIterator::container_type::value_type;
				TP vProxy;
				vProxy.vPos = v;
				(*outVertexIt++) = std::move(vProxy);
			}

			// Push indices
			(*outIndexIt++) = 0;
			(*outIndexIt++) = 1;
			(*outIndexIt++) = 2;
			(*outIndexIt++) = 0;
			(*outIndexIt++) = 2;
			(*outIndexIt++) = 3;
		}

	private:
		glm::vec3 m_aVertices[4];
	};
}