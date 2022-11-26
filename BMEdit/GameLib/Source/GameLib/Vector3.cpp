#include <GameLib/Vector3.h>


using namespace gamelib;

Vector3 Vector3::operator+(const Vector3 &another) const
{
	return Vector3 { x + another.x, y + another.y, z + another.z };
}

Vector3 Vector3::operator-(const gamelib::Vector3 &another) const
{
	return Vector3 { x - another.x, y - another.y, z - another.z };
}

Vector3 Vector3::operator/(float scalar) const
{
	return Vector3 { x / scalar, y / scalar, z / scalar };
}

Vector3 Vector3::operator*(float scalar) const
{
	return Vector3 { x * scalar, y * scalar, z * scalar };
}