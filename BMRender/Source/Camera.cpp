#include <BMRender/Camera.h>
#include <glm/gtc/matrix_transform.hpp>


namespace bmr
{
	Camera::Camera() = default;

	Camera::Camera(float fov, int screenWidth, int screenHeight)
		: m_fov(fov), m_screenSize(screenWidth, screenHeight)
	{
	}

	void Camera::setFOV(float fov)
	{
		m_fov = fov;
		m_isDirty = true;
	}

	void Camera::setCameraPosition(const glm::vec3 &position)
	{
		m_cameraPosition = position;
		m_isDirty = true;
	}

	void Camera::setCameraLookAt(const glm::vec3 &lookAt)
	{
		m_cameraLookAt = lookAt;
		m_isDirty = true;
	}

	void Camera::setCameraHeadPosition(const glm::vec3 &headPos)
	{
		m_cameraHeadPos = headPos;
		m_isDirty = true;
	}

	void Camera::setCameraScreenSize(int width, int height)
	{
		m_screenSize = glm::ivec2 { width, height };
		m_isDirty = true;
	}

	void Camera::setCameraYaw(float yaw)
	{
		m_cameraYaw = yaw;
		m_isDirty = true;
	}

	void Camera::setCameraPitch(float pitch)
	{
		m_cameraPitch = pitch;
		m_isDirty = true;
	}

	void Camera::setCameraRoll(float roll)
	{
		m_cameraRoll = roll;
		m_isDirty = true;
	}

	void Camera::updateCamera() const
	{
		if (m_isDirty)
		{
			m_isDirty = false;

			//TODO: Apply yaw/roll/pitch parameters
			glm::mat4 proj = glm::perspective(glm::radians(m_fov),
			                                  static_cast<float>(m_screenSize.x) / static_cast<float>(m_screenSize.y),
			                                  .0f,
			                                  1000.f);
			glm::mat4 view = glm::lookAt(m_cameraPosition,
			                             m_cameraLookAt,
			                             m_cameraHeadPos);

			m_projView = proj * view;
		}
	}

	float Camera::getFOV() const
	{
		return m_fov;
	}

	const glm::vec3 &Camera::getCameraPosition() const
	{
		return m_cameraPosition;
	}

	const glm::vec3 &Camera::getCameraLookAt() const
	{
		return m_cameraLookAt;
	}

	const glm::vec3 &Camera::getCameraHeadPos() const
	{
		return m_cameraHeadPos;
	}

	float Camera::getCameraYaw() const
	{
		return m_cameraYaw;
	}

	float Camera::getCameraPitch() const
	{
		return m_cameraPitch;
	}

	float Camera::getCameraRoll() const
	{
		return m_cameraRoll;
	}

	const glm::mat4 &Camera::getProjViewMatrix() const
	{
		updateCamera();
		return m_projView;
	}
}