#include <Render/Camera.h>


namespace render
{
	Camera::Camera(float fov, const glm::vec3 &vPosition, const glm::ivec2 &vScreenSize)
	    : m_fFov(fov), m_vPosition(vPosition), m_vScreenSize(vScreenSize)
	{
		update();
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
			if (movementMask & CM_LEFT)     m_vPosition -= m_vRight * fVelocity;
			if (movementMask & CM_RIGHT)    m_vPosition += m_vRight * fVelocity;

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