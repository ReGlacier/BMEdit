#include <GameLib/BoundingBox.h>

using namespace gamelib;


BoundingBox::BoundingBox(const glm::vec3 &vMin, const glm::vec3 &vMax)
	: min(vMin), max(vMax)
{
}

glm::vec3 BoundingBox::getCenter() const
{
	return (min + max) / 2.f;
}