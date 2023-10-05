#pragma once

#include "scene_entity.h"
#include "window.h"

namespace vre
{
	class KeyboardMovementController
	{
	public:
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
		~KeyboardMovementController() = default;

		void applyInput(GLFWwindow* window, float dt, SceneEntity& object);

	private:
		void applyRotation(GLFWwindow* window, float deltaTime, SceneEntity& object);
		void applyMovement(GLFWwindow* window, float deltaTime, SceneEntity& object);

		void toggleMouseRotate(GLFWwindow* window, bool enabled);
		void toogleMousePan(GLFWwindow* window, bool enabled);

		KeyMappings mKeys{};
		float mMoveSpeed{ 2.5f };
		float mLookSpeed{ 1.5f };
		float mMouseSensitivity{ 3.0f };
		glm::vec2 mPreviousCursorPos{0.0f};

		bool mMouseRotateEnabled{ false };
		bool mMousePanEnabled{ false };
	};

} // namespace vre


