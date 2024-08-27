#pragma once

#include "scripting/script_base.h"
#include "utils/math_utils.h"

#include "glm/gtc/matrix_transform.hpp"

namespace Aegix::Scripting
{
	class CameraController : public Scripting::ScriptBase
	{
	public:
		void update(float deltaSeconds) override
		{
			auto& camera = getComponent<Aegix::Component::Camera>();
			auto& transform = getComponent<Aegix::Component::Transform>();

			calcViewMatrix(camera, transform);
			calcProjectionMatrix(camera);
		}

	private:
		void calcViewMatrix(Component::Camera& camera, Component::Transform& transform)
		{
			// Calculate the view matrix based on the camera's transform
			// see: https://www.3dgep.com/understanding-the-view-matrix/
			
			// Vectors for the view coordinate system
			const glm::vec3 f(glm::normalize(transform.forward()));			// Forward
			const glm::vec3 s(glm::normalize(glm::cross(f, MathLib::UP)));	// Sideways
			const glm::vec3 u(glm::cross(s, f));							// Up

			glm::mat4 viewMatrix{ 1.0f };
			viewMatrix[0][0] = s.x;
			viewMatrix[1][0] = s.y;
			viewMatrix[2][0] = s.z;
			viewMatrix[0][1] = u.x;
			viewMatrix[1][1] = u.y;
			viewMatrix[2][1] = u.z;
			viewMatrix[0][2] = -f.x;
			viewMatrix[1][2] = -f.y;
			viewMatrix[2][2] = -f.z;
			viewMatrix[3][0] = -glm::dot(s, transform.location);
			viewMatrix[3][1] = -glm::dot(u, transform.location);
			viewMatrix[3][2] = glm::dot(f, transform.location);
			camera.viewMatrix = viewMatrix;

			glm::mat4 inverseViewMat{ 1.f };
			inverseViewMat[0][0] = s.x;
			inverseViewMat[0][1] = s.y;
			inverseViewMat[0][2] = s.z;
			inverseViewMat[1][0] = u.x;
			inverseViewMat[1][1] = u.y;
			inverseViewMat[1][2] = u.z;
			inverseViewMat[2][0] = f.x;
			inverseViewMat[2][1] = f.y;
			inverseViewMat[2][2] = f.z;
			inverseViewMat[3][0] = transform.location.x;
			inverseViewMat[3][1] = transform.location.y;
			inverseViewMat[3][2] = transform.location.z;
			camera.inverseViewMatrix = inverseViewMat;
		}

		void calcProjectionMatrix(Component::Camera& camera)
		{
			camera.projectionMatrix = glm::perspective(glm::radians(camera.fov), camera.aspectRatio, camera.near, camera.far);
			camera.projectionMatrix[1][1] *= -1; // Correct for Vulkan's inverted Y coordinates
		}
	};
}