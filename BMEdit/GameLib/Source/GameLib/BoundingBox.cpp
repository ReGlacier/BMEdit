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