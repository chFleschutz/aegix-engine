#pragma once

#include "component.h"
#include "window.h"

#include <glm/glm.hpp>

namespace vre
{
	class KeyboardMovementController : public Component
	{
	public:
		using Component::Component;

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

		KeyboardMovementController(GLFWwindow* window);

		void begin() override;
		void update(float deltaSeconds) override;

	private:
		void applyRotation(float deltaSeconds);
		void applyMovement(float deltaSeconds);

		void toggleMouseRotate(bool enabled);
		void toogleMousePan(bool enabled);

		KeyMappings mKeys{};
		float mMoveSpeed{ 2.5f };
		float mLookSpeed{ 1.5f };
		float mMouseSensitivity{ 3.0f };
		glm::vec2 mPreviousCursorPos{0.0f};

		bool mMouseRotateEnabled{ false };
		bool mMousePanEnabled{ false };
	};

} // namespace vre


