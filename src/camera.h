#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace vre
{
	/// @brief Representation of a camera
	/// @note A Camera is created by default when creating a Scene
	class Camera
	{
	public:
		/// @brief Sets the Camera to orthographic projection 
		/// @param Distance to each clipping plane
		void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
		
		/// @brief Sets the Camera to perspective projection
		/// @param fovy Field of view in y-direction
		/// @param aspect The Aspect ratio of the window
		/// @param near Distance to the near clipping plane
		/// @param far Distance to the far clipping plane
		void setPerspectiveProjection(float fovy, float aspect, float near, float far);

		/// @brief Updates the view- and inverse-view-matrix 
		/// @param position Position of the camera
		/// @param direction Direction in which the camera should face
		/// @param up Specifies the rotation of the camera
		void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f));
		
		/// @brief Sets the view direction centered on the target
		/// @param position Position of the camera
		/// @param target Target position which should be in center
		/// @param up Specifies the rotation
		void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f));
		
		/// @brief Sets the view- and inverse-view-matrix based on position and rotation
		/// @param position Position of the camera
		/// @param rotation Rotation of the camera
		void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

		const glm::mat4& projectionMatrix() const { return mProjectionMatrix; }
		const glm::mat4& viewMatrix() const { return mViewMatrix; }
		const glm::mat4& inverseViewMatrix() const { return mInverseViewMatrix; }
		const glm::vec3 position() const { return glm::vec3(mInverseViewMatrix[3]); }

	private:
		glm::mat4 mProjectionMatrix{ 1.0f };
		glm::mat4 mViewMatrix{ 1.0f };
		glm::mat4 mInverseViewMatrix{ 1.0f };
	};

} // namespace vre

