#pragma once

#include <Render/Frustum.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/glm.hpp>

namespace render
{
	using CameraMovementMask = uint8_t;

	enum CameraMovementMaskValues : CameraMovementMask
	{
		CM_FORWARD  = 1 << 0,
		CM_BACKWARD = 1 << 1,
		CM_LEFT     = 1 << 2,
		CM_RIGHT    = 1 << 3,

		CM_SPEEDUP_MOD = 1 << 7
	};

	class Camera
	{
	public:
		static constexpr float kDefaultDt = 1.f / 60.f;

		Camera() = default;
		Camera(float fov, const glm::vec3& vPosition, const glm::ivec2& vScreenSize);

		// Getters
		[[nodiscard]] float getFOV() const { return m_fFov; }
		[[nodiscard]] float getYaw() const { return m_fYaw; }
		[[nodiscard]] float getPitch() const { return m_fPitch; }
		[[nodiscard]] float getSpeed() const { return m_fSpeed; }
		[[nodiscard]] float getSensitivity() const { return m_fSensitivity; }
		[[nodiscard]] const glm::vec3& getPosition() const { return m_vPosition; }
		[[nodiscard]] const glm::vec3& getDirection() const { return m_vLookDirection; }
		[[nodiscard]] const glm::vec3& getUp() const { return m_vUp; }
		[[nodiscard]] const glm::vec3& getWorldUp() const { return m_vWorldUp; }
		[[nodiscard]] const glm::vec3& getRight() const { return m_vRight; }
		[[nodiscard]] const glm::mat4& getView() const { return m_mView; }
		[[nodiscard]] const glm::mat4& getProjection() const { return m_mProj; }
		[[nodiscard]] const glm::mat4& getProjView() const { return m_mProjView; }

		// Setters
		void setFOV(float fov);
		void setSpeed(float speed);
		void setSensitivity(float sens);
		void setViewport(int width, int height);
		void setPosition(const glm::vec3& vPosition);

		// Movement
		void handleKeyboardMovement(CameraMovementMask movementMask = CameraMovementMaskValues::CM_FORWARD, float dt = kDefaultDt);
		void processMouseMovement(float xoffset, float yoffset, float dt = kDefaultDt);
		[[nodiscard]] bool canSeeObject(const glm::vec3& vMin, const glm::vec3& vMax) const;

	private:
		void update();

	private:
		// Camera parameters
		glm::ivec2 m_vScreenSize { 0 };
		float m_fFov { 80.f };
		float m_fYaw { -90.f };
		float m_fPitch { .0f };
		float m_fSpeed { 2.5f };
		float m_fSensitivity { 0.1f };
		float m_fNearPlane { .01f };
		float m_fFarPlane { 10'000.f };

		// Calculated things
		glm::vec3 m_vPosition { .0f };
		glm::vec3 m_vLookDirection { .0f, .0f, -1.f };
		glm::vec3 m_vUp { .0f, 1.f, .0f };
		glm::vec3 m_vRight { .0f };
		glm::vec3 m_vWorldUp { .0f, 1.0f, .0f };  // On init same to m_vUp

		glm::mat4 m_mView { 1.f };
		glm::mat4 m_mProj { 1.f };
		glm::mat4 m_mProjView { 1.f };

		// Frustum
		Frustum m_sFrustum {};
	};
}