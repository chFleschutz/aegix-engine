#pragma once

#include "core/input.h"
#include "scripting/script_base.h"

namespace VEScripting
{
	class DynamicMovementController : public ScriptBase
	{
	public:
		struct KeyMappings
		{
			Input::Key moveForward = Input::Up;
			Input::Key moveBackward = Input::Down;
			Input::Key rotateLeft = Input::Left;
			Input::Key rotateRight = Input::Right;
		};

		virtual void update(float deltaSeconds) override;
		
	private:
		KeyMappings m_keys{};

		float m_dragFactor = 2.0f;
		float m_acceleration = 2.0f;
	};

} // namespace VEScripting