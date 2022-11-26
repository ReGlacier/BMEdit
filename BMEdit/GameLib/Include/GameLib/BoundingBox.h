#pragma once

#include <GameLib/Vector3.h>


namespace gamelib
{
	struct BoundingBox
	{
		Vector3 min;
		Vector3 max;

		BoundingBox() = default;
		BoundingBox(const Vector3 &vMin, const Vector3 &vMax);

		Vector3 getCenter() const;
	};
}