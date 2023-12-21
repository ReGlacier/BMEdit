#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/glm.hpp>


namespace render
{
	// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
	enum Camera_Movement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	// Default camera values
	const float YAW         = -90.0f;
	const float PITCH       =  0.0f;
	const float SPEED       =  2.5f;
	const float SENSITIVITY =  0.1f;
	const float ZOOM        =  45.0f;

	/**
	 * @credits https://learnopengl.com/Getting-started/Camera
	 */
	class Camera
	{
	public:
		// camera Attributes
		glm::vec3 Position;
		glm::vec3 Front;
		glm::vec3 Up;
		glm::vec3 Right;
		glm::vec3 WorldUp;
		// euler Angles
		float Yaw;
		float Pitch;
		// camera options
		float MovementSpeed;
		float MouseSensitivity;
		float Zoom;

		// constructor with vectors
		Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
		{
			Position = position;
			WorldUp = up;
			Yaw = yaw;
			Pitch = pitch;
			updateCameraVectors();
		}

		// constructor with scalar values
		Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
		{
			Position = glm::vec3(posX, posY, posZ);
			WorldUp = glm::vec3(upX, upY, upZ);
			Yaw = yaw;
			Pitch = pitch;
			updateCameraVectors();
		}

		void setPosition(const glm::vec3& position)
		{
			Position = position;
			updateCameraVectors();
		}

		// returns the view matrix calculated using Euler Angles and the LookAt Matrix
		glm::mat4 getViewMatrix() const
		{
			return glm::lookAt(Position, Position + Front, Up);
		}

		// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
		void processKeyboard(Camera_Movement direction, float deltaTime, float moveScale = 1.f)
		{
			float velocity = MovementSpeed * deltaTime * moveScale;
			if (direction == FORWARD)
				Position -= Front * velocity;
			if (direction == BACKWARD)
				Position += Front * velocity;
			if (direction == LEFT)
				Position -= Right * velocity;
			if (direction == RIGHT)
				Position += Right * velocity;
		}

		// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
		void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
		{
			xoffset *= MouseSensitivity;
			yoffset *= MouseSensitivity;

			Yaw   += xoffset;
			Pitch += yoffset;

			// make sure that when pitch is out of bounds, screen doesn't get flipped
			if (constrainPitch)
			{
				if (Pitch > 89.0f)
					Pitch = 89.0f;
				if (Pitch < -89.0f)
					Pitch = -89.0f;
			}

			// update Front, Right and Up Vectors using the updated Euler angles
			updateCameraVectors();
		}

		// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
		void processMouseScroll(float yoffset)
		{
			Zoom -= (float)yoffset;
			if (Zoom < 1.0f)
				Zoom = 1.0f;
			if (Zoom > 45.0f)
				Zoom = 45.0f;
		}

		// return true if object represented by vMin & vMax is visible
		[[nodiscard]] bool canSeeObject(const glm::vec3& vMin, const glm::vec3& vMax, const glm::mat4& mProj) const
		{
			return true;
			/*
			 * // still buggy shit
			const bool bCenter = glm::dot(Front, Position - ((vMax - vMin) * .5f)) > .0f;
			const bool bMin = glm::dot(Front, Position - vMin) > .0f;
			const bool bMax = glm::dot(Front, Position - vMax) > .0f;
			return bCenter || bMin || bMax;
		    */
		}

	private:
		// calculates the front vector from the Camera's (updated) Euler Angles
		void updateCameraVectors()
		{
			// calculate the new Front vector
			glm::vec3 front;
			front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
			front.y = sin(glm::radians(Pitch));
			front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
			Front = glm::normalize(front);
			// also re-calculate the Right and Up vector
			Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
			Up    = glm::normalize(glm::cross(Right, Front));
		}
	};
}