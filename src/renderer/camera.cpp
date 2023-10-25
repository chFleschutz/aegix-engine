#include "camera.h"

#include <cassert>
#include <limits>

#include <iostream>

namespace vre
{
	void Camera::setOrthographicProjection(float left, float right, float top, float bottom, float near, float far)
	{
		m_projectionMatrix = Matrix4{ 1.0f };
		m_projectionMatrix[0][0] = 2.f / (right - left);
		m_projectionMatrix[1][1] = 2.f / (bottom - top);
		m_projectionMatrix[2][2] = 1.f / (far - near);
		m_projectionMatrix[3][0] = -(right + left) / (right - left);
		m_projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
		m_projectionMatrix[3][2] = -near / (far - near);
	}

	void Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far)
	{
		assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);

		const float tanHalfFovy = tan(fovy / 2.f);
		m_projectionMatrix = Matrix4{ 0.0f };
		m_projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
		m_projectionMatrix[1][1] = 1.f / (tanHalfFovy);
		m_projectionMatrix[2][2] = far / (far - near);
		m_projectionMatrix[2][3] = 1.f;
		m_projectionMatrix[3][2] = -(far * near) / (far - near);
	}

	void Camera::setViewDirection(Vector3 position, Vector3 direction, Vector3 up)
	{
		assert(glm::abs(glm::length(direction) - std::numeric_limits<float>::epsilon()) > 0.0f);

		const Vector3 w{glm::normalize(direction)};
		const Vector3 u{glm::normalize(glm::cross(w, up))};
		const Vector3 v{glm::cross(w, u)};

		m_viewMatrix = Matrix4{ 1.f };
		m_viewMatrix[0][0] = u.x;
		m_viewMatrix[1][0] = u.y;
		m_viewMatrix[2][0] = u.z;
		m_viewMatrix[0][1] = v.x;
		m_viewMatrix[1][1] = v.y;
		m_viewMatrix[2][1] = v.z;
		m_viewMatrix[0][2] = w.x;
		m_viewMatrix[1][2] = w.y;
		m_viewMatrix[2][2] = w.z;
		m_viewMatrix[3][0] = -glm::dot(u, position);
		m_viewMatrix[3][1] = -glm::dot(v, position);
		m_viewMatrix[3][2] = -glm::dot(w, position);

		m_inverseViewMatrix = Matrix4{ 1.f };
		m_inverseViewMatrix[0][0] = u.x;
		m_inverseViewMatrix[0][1] = u.y;
		m_inverseViewMatrix[0][2] = u.z;
		m_inverseViewMatrix[1][0] = v.x;
		m_inverseViewMatrix[1][1] = v.y;
		m_inverseViewMatrix[1][2] = v.z;
		m_inverseViewMatrix[2][0] = w.x;
		m_inverseViewMatrix[2][1] = w.y;
		m_inverseViewMatrix[2][2] = w.z;
		m_inverseViewMatrix[3][0] = position.x;
		m_inverseViewMatrix[3][1] = position.y;
		m_inverseViewMatrix[3][2] = position.z;
	}

	void Camera::setViewTarget(Vector3 position, Vector3 target, Vector3 up)
	{
		setViewDirection(position, target - position, up);
	}

	void Camera::setViewYXZ(Vector3 position, Vector3 rotation)
	{
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const Vector3 u{(c1* c3 + s1 * s2 * s3), (c2* s3), (c1* s2* s3 - c3 * s1)};
		const Vector3 v{(c3* s1* s2 - c1 * s3), (c2* c3), (c1* c3* s2 + s1 * s3)};
		const Vector3 w{(c2* s1), (-s2), (c1* c2)};
		m_viewMatrix = Matrix4{ 1.0f };
		m_viewMatrix[0][0] = u.x;
		m_viewMatrix[1][0] = u.y;
		m_viewMatrix[2][0] = u.z;
		m_viewMatrix[0][1] = v.x;
		m_viewMatrix[1][1] = v.y;
		m_viewMatrix[2][1] = v.z;
		m_viewMatrix[0][2] = w.x;
		m_viewMatrix[1][2] = w.y;
		m_viewMatrix[2][2] = w.z;
		m_viewMatrix[3][0] = -glm::dot(u, position);
		m_viewMatrix[3][1] = -glm::dot(v, position);
		m_viewMatrix[3][2] = -glm::dot(w, position);

		m_inverseViewMatrix = Matrix4{ 1.f };
		m_inverseViewMatrix[0][0] = u.x;
		m_inverseViewMatrix[0][1] = u.y;
		m_inverseViewMatrix[0][2] = u.z;
		m_inverseViewMatrix[1][0] = v.x;
		m_inverseViewMatrix[1][1] = v.y;
		m_inverseViewMatrix[1][2] = v.z;
		m_inverseViewMatrix[2][0] = w.x;
		m_inverseViewMatrix[2][1] = w.y;
		m_inverseViewMatrix[2][2] = w.z;
		m_inverseViewMatrix[3][0] = position.x;
		m_inverseViewMatrix[3][1] = position.y;
		m_inverseViewMatrix[3][2] = position.z;
	}

} // namespace vre