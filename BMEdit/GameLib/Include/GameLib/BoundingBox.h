#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>


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
	};
}