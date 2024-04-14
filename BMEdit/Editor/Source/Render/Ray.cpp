#include <Render/Ray.h>


namespace render
{
	bool Ray::intersect(const gamelib::BoundingBox& boundingBox, bool bAllowRaysStartingInsideTargetBbox) const
	{
		const float t1 = (boundingBox.min.x - vOrigin.x) / vDirection.x;
		const float t2 = (boundingBox.max.x - vOrigin.x) / vDirection.x;
		const float t3 = (boundingBox.min.y - vOrigin.y) / vDirection.y;
		const float t4 = (boundingBox.max.y - vOrigin.y) / vDirection.y;
		const float t5 = (boundingBox.min.z - vOrigin.z) / vDirection.z;
		const float t6 = (boundingBox.max.z - vOrigin.z) / vDirection.z;

		const float tMin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
		const float tMax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

		if (tMax < .0f || tMin > tMax || (!bAllowRaysStartingInsideTargetBbox && tMin < 0.f))
		{
			return false;
		}

		return true;
	}
}