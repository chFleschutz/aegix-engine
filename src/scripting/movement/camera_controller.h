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
			calcPerspectiveProjection(camera);
		}

	private:
		void calcViewMatrix(Component::Camera& camera, Component::Transform& transform);
		void calcPerspectiveProjection(Component::Camera& camera);
	};
}