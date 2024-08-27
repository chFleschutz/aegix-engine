#pragma once

#include "scripting/script_base.h"

namespace Aegix::Scripting
{
	class CameraController : public Scripting::ScriptBase
	{
	public:
		void update(float deltaSeconds) override
		{
			auto& camera = getComponent<Aegix::Component::Camera>();
			auto& transform = getComponent<Aegix::Component::Transform>();
			camera.camera.setViewYXZ(transform.location, transform.rotation);
		}
	};
}