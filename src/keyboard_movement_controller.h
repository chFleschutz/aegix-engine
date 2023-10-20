#pragma once

#include "utils/math_utils.h"
#include "renderer/window.h"
#include "scene/script_component_base.h"

namespace vre
{
	class KeyboardMovementController : public ScriptComponentBase
	{
	public:
		/// @brief Default keybindings for movement and view control
		struct KeyMappings
		{
			int moveLeft = GLFW_KEY_A;
			int moveRight = GLFW_KEY_D;
			int moveForward = GLFW_KEY_W;
			int moveBackward = GLFW_KEY_S;
			int moveUp = GLFW_KEY_E;
			int moveDown = GLFW_KEY_Q;

			int lookLeft = GLFW_KEY_LEFT;
			int lookRight = GLFW_KEY_RIGHT;
			int lookUp = GLFW_KEY_UP;
			int lookDown = GLFW_KEY_DOWN;

			int mouseRotate = GLFW_MOUSE_BUTTON_RIGHT;
			int mousePan = GLFW_MOUSE_BUTTON_MIDDLE;
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
		float m_moveSpeed{ 2.5f };
		float m_lookSpeed{ 1.5f };
		float m_mouseSensitivity{ 3.0f };
		glm::vec2 m_previousCursorPos{0.0f};

		bool m_mouseRotateEnabled{ false };
		bool m_mousePanEnabled{ false };
	};

} // namespace vre


