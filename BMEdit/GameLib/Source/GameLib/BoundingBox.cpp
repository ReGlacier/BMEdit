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
	expand(another.min);
	expand(another.max);
}

void BoundingBox::expand(const glm::vec3 &vPoint)
{
	min.x = std::min(min.x, vPoint.x);
	min.y = std::min(min.y, vPoint.y);
	min.z = std::min(min.z, vPoint.z);
	max.x = std::max(max.x, vPoint.x);
	max.y = std::max(max.y, vPoint.y);
	max.z = std::max(max.z, vPoint.z);
}

bool BoundingBox::contains(const glm::vec3& vPoint) const
{
	return  vPoint.x >= min.x && vPoint.x <= max.x &&
			vPoint.y >= min.y && vPoint.y <= max.y &&
			vPoint.z >= min.z && vPoint.z <= max.z;
}

bool BoundingBox::intersect(const gamelib::BoundingBox& another) const
{
	if (min.x > another.max.x) return false;
	if (max.x < another.min.x) return false;
	if (min.y > another.max.y) return false;
	if (max.y < another.min.y) return false;
	if (min.z > another.max.z) return false;
	if (max.z < another.min.z) return false;

	return true;
}

double BoundingBox::getVolume() const
{
	static auto w = static_cast<double>(max.x - min.x);
	static auto h = static_cast<double>(max.y - min.y);
	static auto d = static_cast<double>(max.z - min.z);

	return w * h * d;
}

std::tuple<float, float, float> BoundingBox::getDimensions() const
{
	return std::make_tuple(
		max.x - min.x,
		max.y - min.y,
		max.z - min.z
	);
}

BoundingBox BoundingBox::toWorld(const BoundingBox& source, const glm::mat4& mTransform)
{
	glm::vec3 vMin = source.min;
	glm::vec3 vMax = source.max;

	glm::vec3 avVertices[8];
	avVertices[0] = vMin;
	avVertices[1] = glm::vec3(vMax.x, vMin.y, vMin.z);
	avVertices[2] = glm::vec3(vMin.x, vMax.y, vMin.z);
	avVertices[3] = glm::vec3(vMax.x, vMax.y, vMin.z);
	avVertices[4] = glm::vec3(vMin.x, vMin.y, vMax.z);
	avVertices[5] = glm::vec3(vMax.x, vMin.y, vMax.z);
	avVertices[6] = glm::vec3(vMin.x, vMax.y, vMax.z);
	avVertices[7] = vMax;

	BoundingBox result {};
	result.min = glm::vec3(mTransform * glm::vec4(avVertices[0], 1.f));
	result.max = result.min;

	for (int i = 1; i < 8; i++)
	{
		glm::vec3 vTransformed = glm::vec3(mTransform * glm::vec4(avVertices[i], 1.f));
		result.min = glm::min(result.min, vTransformed);
		result.max = glm::max(result.max, vTransformed);
	}

	return result;
}