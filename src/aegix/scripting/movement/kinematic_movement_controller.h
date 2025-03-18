#pragma once

#include "core/input.h"
#include "math/math.h"
#include "scripting/script_base.h"

namespace Aegix::Scripting
{
	class KinematcMovementController : public ScriptBase
	{
	public:
		/// @brief Default keybindings for movement and view control
		struct KeyMappings
		{
			Input::Key moveLeft = Input::A;
			Input::Key moveRight = Input::D;
			Input::Key moveForward = Input::W;
			Input::Key moveBackward = Input::S;
			Input::Key moveUp = Input::E;
			Input::Key moveDown = Input::Q;

			Input::MouseButton mouseRotate = Input::MouseRight;
			Input::MouseButton mousePan = Input::MouseMiddle;

			// Disabled by default
			Input::Key lookLeft = Input::Unknown;
			Input::Key lookRight = Input::Unknown;
			Input::Key lookUp = Input::Unknown;
			Input::Key lookDown = Input::Unknown;
		};

		void begin() override;
		void update(float deltaSeconds) override;

		/// @brief Sets new overall move speed
		void setMoveSpeed(float speed) { m_moveSpeed = speed; }
		/// @brief Sets new overall look speed
		void setLookSpeed(float speed) { m_lookSpeed = speed; }
		/// @brief Sets speed for looking with the mouse
		void setMouseSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }

	private:
		void applyRotation(float deltaSeconds);
		void applyMovement(float deltaSeconds);

		void toggleMouseRotate(bool enabled);
		void toogleMousePan(bool enabled);

		KeyMappings m_keys{};
		float m_moveSpeed{ 5.0f };
		float m_lookSpeed{ 1.5f };
		float m_mouseSensitivity{ 0.25f };
		bool m_mouseRotateEnabled{ false };
		bool m_mousePanEnabled{ false };

		glm::vec2 m_previousCursorPos{ 0.0f };
	};
}
