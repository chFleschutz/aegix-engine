#pragma once

#include "core/input.h"
#include "scripting/script_base.h"

namespace Aegis::Scripting
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

		float m_linearforce = 20.0f;
		float m_angularforce = 20.0f;
	};
}
