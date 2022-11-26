#include <GameLib/BoundingBox.h>

using namespace gamelib;


BoundingBox::BoundingBox(const gamelib::Vector3 &vMin, const gamelib::Vector3 &vMax)
	: min(vMin), max(vMax)
{
}

Vector3 BoundingBox::getCenter() const
{
	return (min + max) / 2.f;
}