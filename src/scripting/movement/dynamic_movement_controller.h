#pragma once

#include "scripting/script_base.h"

namespace VEScripting
{
	class DynamicMovementController : public ScriptBase
	{
	public:
		struct KeyMappings
		{
			int moveForward = GLFW_KEY_W;
			int moveBackward = GLFW_KEY_S;
			int rotateLeft = GLFW_KEY_A;
			int rotateRight = GLFW_KEY_D;
		};

		virtual void update(float deltaSeconds) override;
		
	private:
		KeyMappings m_keys{};

		float m_dragFactor = 0.5f;
		float m_acceleration = 2.0f;
	};

} // namespace VEScripting