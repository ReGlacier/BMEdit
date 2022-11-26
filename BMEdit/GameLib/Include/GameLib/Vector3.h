#pragma once


namespace gamelib
{
	struct Vector3
	{
		float x { .0f };
		float y { .0f };
		float z { .0f };

		Vector3 operator+(const Vector3 &another) const;
		Vector3 operator-(const Vector3 &another) const;
		Vector3 operator/(float scalar) const;
		Vector3 operator*(float scalar) const;
	};
}