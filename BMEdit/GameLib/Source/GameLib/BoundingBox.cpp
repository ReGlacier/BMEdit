#include <GameLib/BoundingBox.h>
#include <algorithm>

using namespace gamelib;


BoundingBox::BoundingBox(const glm::vec3 &vMin, const glm::vec3 &vMax)
	: min(vMin), max(vMax)
{
}

glm::vec3 BoundingBox::getCenter() const
{
	return (min + max) / 2.f;
}

void BoundingBox::expand(const BoundingBox& another)
{
	min.x = std::min(min.x, another.min.x);
	min.y = std::min(min.y, another.min.y);
	min.z = std::min(min.z, another.min.z);
	max.x = std::max(max.x, another.max.x);
	max.y = std::max(max.y, another.max.y);
	max.z = std::max(max.z, another.max.z);
}

bool BoundingBox::contains(const glm::vec3& vPoint) const
{
	return  vPoint.x >= min.x && vPoint.x <= max.x &&
			vPoint.y >= min.y && vPoint.y <= max.y &&
			vPoint.z >= min.z && vPoint.z <= max.z;
}