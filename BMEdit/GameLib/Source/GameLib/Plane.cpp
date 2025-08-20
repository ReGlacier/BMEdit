#include <GameLib/Plane.h>


namespace gamelib
{
	Plane::Plane() = default;

	Plane::Plane(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3)
	{
		m_aVertices[0] = v0;
		m_aVertices[1] = v1;
		m_aVertices[2] = v2;
		m_aVertices[3] = v3;
	}

	BoundingBox Plane::makeBoundingBox() const
	{
		glm::vec3 vMin = m_aVertices[0];
		glm::vec3 vMax = m_aVertices[0];

		for (int i = 1; i < 4; i++)
		{
			vMin = glm::min(vMin, m_aVertices[i]);
			vMax = glm::min(vMax, m_aVertices[i]);
		}

		return { vMin, vMax };
	}

	glm::vec3 Plane::getNormal() const
	{
		glm::vec3 u = m_aVertices[1] - m_aVertices[0];
		glm::vec3 v = m_aVertices[2] - m_aVertices[0];
		glm::vec3 vNormal = glm::cross(u, v);
		return glm::normalize(vNormal);
	}

	glm::vec3 Plane::getBiNormal() const
	{
		return -getNormal();
	}

	glm::vec3 Plane::getCenter() const
	{
		glm::vec3 center { .0f };

		for (const auto& vPoint : m_aVertices) {
			center += vPoint;
		}

		center /= 4.0f;
		return center;
	}

	const glm::vec3& Plane::getPoint(size_t idx) const
	{
		if (idx >= 0 && idx <= 3)
		{
			return m_aVertices[idx];
		}

		static const glm::vec3 kNull { 0.f };
		return kNull;
	}

	float Plane::getSize() const
	{
		float fMaxDistance = .0f;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				if (i == j)
					continue;

				float fDistance = glm::distance(m_aVertices[i], m_aVertices[j]);
				fMaxDistance = std::max(fMaxDistance, fDistance);
			}
		}

		return fMaxDistance;
	}
}