#pragma once

#include <glm/vec3.hpp>


namespace gamelib
{
	struct BoundingBox
	{
		glm::vec3 min;
		glm::vec3 max;

		BoundingBox() = default;
		BoundingBox(const glm::vec3 &vMin, const glm::vec3 &vMax);

		glm::vec3 getCenter() const;
	};
}