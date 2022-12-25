#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>


namespace bmr
{
	class Camera
	{
	public:
		Camera();
		Camera(float fov, int screenWidth, int screenHeight);

		void setFOV(float fov);
		void setCameraPosition(const glm::vec3 &position);
		void setCameraLookAt(const glm::vec3 &lookAt);
		void setCameraHeadPosition(const glm::vec3 &headPos);
		void setCameraScreenSize(int width, int height);
		void setCameraYaw(float yaw);
		void setCameraPitch(float pitch);
		void setCameraRoll(float roll);

		[[nodiscard]] float getFOV() const;
		[[nodiscard]] const glm::vec3 &getCameraPosition() const;
		[[nodiscard]] const glm::vec3 &getCameraLookAt() const;
		[[nodiscard]] const glm::vec3 &getCameraHeadPos() const;
		[[nodiscard]] float getCameraYaw() const;
		[[nodiscard]] float getCameraPitch() const;
		[[nodiscard]] float getCameraRoll() const;
		[[nodiscard]] const glm::mat4 &getProjViewMatrix() const;

	private:
		void updateCamera() const;

	private:
		mutable glm::mat4 m_projView{1.f};
		glm::vec3 m_cameraPosition{.0f, .0f, .0f};
		glm::vec3 m_cameraLookAt{.0f, .0f, .0f};
		glm::vec3 m_cameraHeadPos{.0f, 1.f, .0f};
		float m_fov{45.f};
		float m_cameraYaw{.0f};
		float m_cameraPitch{.0f};
		float m_cameraRoll{.0f};
		glm::ivec2 m_screenSize{0, 0};
		mutable bool m_isDirty{false};
	};
}