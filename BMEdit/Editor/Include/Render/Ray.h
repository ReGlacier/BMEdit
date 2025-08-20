#pragma once

#include <GameLib/BoundingBox.h>
#include <glm/vec3.hpp>


namespace render
{
	struct Ray
	{
		glm::vec3 vOrigin { .0f };
		glm::vec3 vDirection { .0f };

		[[nodiscard]] bool intersect(const gamelib::BoundingBox& boundingBox, bool bAllowRaysStartingInsideTargetBbox) const;
	};
}