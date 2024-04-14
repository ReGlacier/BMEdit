#pragma once

#include <glm/glm.hpp>
#include <array>


namespace render
{
	class Frustum
	{
	public:
		Frustum() = default;

		void setup(const glm::mat4& mProjView)
		{
			m_vPlanes[0] = glm::vec4(mProjView[0][3] + mProjView[0][0], mProjView[1][3] + mProjView[1][0], mProjView[2][3] + mProjView[2][0], mProjView[3][3] + mProjView[3][0]);
			m_vPlanes[1] = glm::vec4(mProjView[0][3] - mProjView[0][0], mProjView[1][3] - mProjView[1][0], mProjView[2][3] - mProjView[2][0], mProjView[3][3] - mProjView[3][0]);
			m_vPlanes[2] = glm::vec4(mProjView[0][3] + mProjView[0][1], mProjView[1][3] + mProjView[1][1], mProjView[2][3] + mProjView[2][1], mProjView[3][3] + mProjView[3][1]);
			m_vPlanes[3] = glm::vec4(mProjView[0][3] - mProjView[0][1], mProjView[1][3] - mProjView[1][1], mProjView[2][3] - mProjView[2][1], mProjView[3][3] - mProjView[3][1]);
			m_vPlanes[4] = glm::vec4(mProjView[0][3] + mProjView[0][2], mProjView[1][3] + mProjView[1][2], mProjView[2][3] + mProjView[2][2], mProjView[3][3] + mProjView[3][2]);
			m_vPlanes[5] = glm::vec4(mProjView[0][3] - mProjView[0][2], mProjView[1][3] - mProjView[1][2], mProjView[2][3] - mProjView[2][2], mProjView[3][3] - mProjView[3][2]);

			for (auto& vPlane : m_vPlanes)
			{
				float fLen = glm::length(glm::vec3(vPlane));
				vPlane /= fLen;
			}
		}

		[[nodiscard]] bool isBoxVisible(const glm::vec3& vMin, const glm::vec3& vMax) const
		{
			for (const auto& vPlane : m_vPlanes)
			{
				if (vPlane.x * (vPlane.x > 0 ? vMax.x : vMin.x) + vPlane.y * (vPlane.y > 0 ? vMax.y : vMin.y) + vPlane.z * (vPlane.z > 0 ? vMax.z : vMin.z) + vPlane.w <= 0)
				{
					return false;
				}
			}
			return true;
		}

	private:
		std::array<glm::vec4, 6> m_vPlanes;
	};
}