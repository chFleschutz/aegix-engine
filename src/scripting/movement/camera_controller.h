#pragma once

#include "scripting/script_base.h"

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
			auto forward = transform.forward();
			auto up = glm::vec3{ 0.0f, 0.0f, 1.0f };
			camera.viewMatrix = glm::lookAt(transform.location, transform.location + forward, up);
			camera.inverseViewMatrix = glm::inverse(camera.viewMatrix);
		}

		void calcProjectionMatrix(Component::Camera& camera)
		{
			camera.projectionMatrix = glm::perspective(glm::radians(camera.fov), camera.aspectRatio, camera.near, camera.far);
			camera.projectionMatrix[1][1] *= -1; // Correct for Vulkan's inverted Y coordinates
		}
	};
}