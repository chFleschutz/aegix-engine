#pragma once

#include "utils/math_utils.h"

namespace VEGraphics
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
		void setViewDirection(Vector3 position, Vector3 direction, Vector3 up = Vector3(0.0f, -1.0f, 0.0f));
		
		/// @brief Sets the view direction centered on the target
		/// @param position Position of the camera
		/// @param target Target position which should be in center
		/// @param up Specifies the rotation
		void setViewTarget(Vector3 position, Vector3 target, Vector3 up = Vector3(0.0f, -1.0f, 0.0f));
		
		/// @brief Sets the view- and inverse-view-matrix based on position and rotation
		/// @param position Position of the camera
		/// @param rotation Rotation of the camera
		void setViewYXZ(Vector3 position, Vector3 rotation);

		const Matrix4& projectionMatrix() const { return m_projectionMatrix; }
		const Matrix4& viewMatrix() const { return m_viewMatrix; }
		const Matrix4& inverseViewMatrix() const { return m_inverseViewMatrix; }
		const Vector3 position() const { return Vector3(m_inverseViewMatrix[3]); }

	private:
		Matrix4 m_projectionMatrix{ 1.0f };
		Matrix4 m_viewMatrix{ 1.0f };
		Matrix4 m_inverseViewMatrix{ 1.0f };
	};

} // namespace vre

