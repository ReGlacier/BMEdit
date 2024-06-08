#include <Render/Camera.h>


namespace render
{
	Camera::Camera(float fov, const glm::vec3 &vPosition, const glm::ivec2 &vScreenSize)
	    : m_fFov(fov), m_vPosition(vPosition), m_vScreenSize(vScreenSize)
	{
		update();
	}

	Ray Camera::getRayFromScreen(const glm::ivec2& vScreenPos) const
	{
		return getRayFromScreen(static_cast<float>(vScreenPos.x), static_cast<float>(vScreenPos.y));
	}

	Ray Camera::getRayFromScreen(float x, float y) const
	{
		constexpr float kSign = 1.f;
		glm::vec4 vRayClip = glm::vec4((2.f * x) / static_cast<float>(m_vScreenSize.x) - 1.f, 1.f - (2.f * y) / static_cast<float>(m_vScreenSize.y), kSign, 1.f);
		glm::vec4 vRayEye = glm::inverse(m_mProj) * vRayClip;
		vRayEye = glm::vec4 { vRayEye.x, vRayEye.y, kSign, .0f };
		glm::vec3 vRayWorld = glm::normalize(glm::vec3(glm::inverse(m_mView) * vRayEye));
		return { getPosition(), vRayWorld };
	}

	void Camera::setFOV(float fov)
	{
		if (m_fFov != fov)
		{
			m_fFov = fov;
			update();
		}
	}

	void Camera::setSpeed(float speed)
	{
		if (speed > 0.f)
		{
			m_fSpeed = speed;
		}
	}

	void Camera::setSensitivity(float sens)
	{
		if (sens > .0f)
		{
			m_fSensitivity = sens;
		}
	}

	void Camera::setViewport(int width, int height)
	{
		if (m_vScreenSize.x != width || m_vScreenSize.y != height)
		{
			m_vScreenSize.x = width;
			m_vScreenSize.y = height;

			update();
		}
	}

	void Camera::setPosition(const glm::vec3& vPosition)
	{
		m_vPosition = vPosition;
		update();
	}

	// Movement
	void Camera::handleKeyboardMovement(CameraMovementMask movementMask, float dt)
	{
		// Handle keyboard movement
		const float fSpeedUp = (movementMask & CM_SPEEDUP_MOD) ? 4.0f : 1.0f;
		const float fVelocity = m_fSpeed * fSpeedUp;

		if ((movementMask & CM_FORWARD) && (movementMask & CM_BACKWARD)) movementMask &= ~(CM_FORWARD | CM_BACKWARD);
		if ((movementMask & CM_LEFT) && (movementMask & CM_RIGHT)) movementMask &= ~(CM_LEFT | CM_RIGHT);
		if (movementMask == CM_SPEEDUP_MOD) movementMask = 0;

		if (movementMask > 0) {
			if (movementMask & CM_FORWARD)  m_vPosition += m_vLookDirection * fVelocity;
			if (movementMask & CM_BACKWARD) m_vPosition -= m_vLookDirection * fVelocity;
			if (movementMask & CM_LEFT)     m_vPosition += m_vRight * fVelocity;
			if (movementMask & CM_RIGHT)    m_vPosition -= m_vRight * fVelocity;

			update();
		}
	}

	void Camera::processMouseMovement(float xoffset, float yoffset, float dt)
	{
		// Handle mouse movement
		xoffset *= m_fSensitivity;
		yoffset *= m_fSensitivity;

		m_fYaw += xoffset;
		m_fPitch += yoffset;

		if (m_fPitch > 89.0f)
			m_fPitch = 89.0f;

		if (m_fPitch < -89.0f)
			m_fPitch = -89.0f;

		update();
	}

	bool Camera::canSeeObject(const glm::vec3& vMin, const glm::vec3& vMax) const
	{
		return m_sFrustum.isBoxVisible(vMin, vMax);
	}

	bool Camera::canSeeObject(const gamelib::BoundingBox& bbox) const
	{
		return glm::distance(m_vPosition, bbox.getCenter()) <= m_fFarPlane && m_sFrustum.isBoxVisible(bbox.min, bbox.max);
	}

	bool Camera::canSeeObject(const gamelib::Plane& plane) const
	{
		const glm::vec3 vU = plane.getPoint(1) - plane.getPoint(0);
		const glm::vec3 vV = plane.getPoint(2) - plane.getPoint(0);
		const glm::vec3 vNormal = glm::cross(vU, vV);

		if (glm::dot(vNormal, m_vLookDirection) >= 0) {
			return false;
		}

		return true; // NOTE: Maybe we really need to check this, but it works well for now
		//return m_sFrustum.isPlaneVisible(plane);
	}

	void Camera::update()
	{
		glm::vec3 vFront { .0f };
		vFront.x = cos(glm::radians(m_fYaw)) * cos(glm::radians(m_fPitch));
		vFront.y = sin(glm::radians(m_fPitch));
		vFront.z = sin(glm::radians(m_fYaw)) * cos(glm::radians(m_fPitch));
		m_vLookDirection = glm::normalize(vFront);

		m_vRight = glm::normalize(glm::cross(m_vLookDirection, m_vWorldUp));
		m_vUp = glm::normalize(glm::cross(m_vRight, m_vLookDirection));

		m_mView = glm::lookAtLH(m_vPosition, m_vPosition + m_vLookDirection, m_vUp);
		m_mProj = glm::perspectiveFovLH(glm::radians(m_fFov), static_cast<float>(m_vScreenSize.x), static_cast<float>(m_vScreenSize.y), m_fNearPlane, m_fFarPlane);
		m_mProjView = m_mProj * m_mView;

		m_sFrustum.setup(m_mProjView);
	}
}