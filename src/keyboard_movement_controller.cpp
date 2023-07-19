#include "keyboard_movement_controller.h"

#include <limits>

namespace vre
{
	KeyboardMovementController::KeyboardMovementController(GLFWwindow* window)
	{

		if (glfwRawMouseMotionSupported())
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

		double cursorXPos, cursorYPos;
		glfwGetCursorPos(window, &cursorXPos, &cursorYPos);
		mPreviousCursorPos = { cursorXPos,cursorYPos };
	}

	void KeyboardMovementController::applyInput(GLFWwindow* window, float deltaTime, VreSceneObject& object)
	{
		applyRotation(window, deltaTime, object);
		applyMovement(window, deltaTime, object);
	}

	void KeyboardMovementController::applyRotation(GLFWwindow* window, float deltaTime, VreSceneObject& object)
	{
		// Key input rotation
		glm::vec3 rotate{0.0f};
		if (glfwGetKey(window, mKeys.lookRight) == GLFW_PRESS) rotate.y += 1.0f;
		if (glfwGetKey(window, mKeys.lookLeft) == GLFW_PRESS) rotate.y -= 1.0f;
		if (glfwGetKey(window, mKeys.lookUp) == GLFW_PRESS) rotate.x += 1.0f;
		if (glfwGetKey(window, mKeys.lookDown) == GLFW_PRESS) rotate.x -= 1.0f;

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
			object.transform.rotation += mLookSpeed * glm::normalize(rotate) * deltaTime;

		// Mouse input rotation
		toggleMouseRotate(window, glfwGetMouseButton(window, mKeys.mouseRotate) == GLFW_PRESS);
		if (mMouseRotateEnabled)
		{
			double cursorXPos, cursorYPos;
			glfwGetCursorPos(window, &cursorXPos, &cursorYPos);

			rotate.x -= static_cast<float>(cursorYPos) - mPreviousCursorPos.y;
			rotate.y += static_cast<float>(cursorXPos) - mPreviousCursorPos.x;
			mPreviousCursorPos = { cursorXPos, cursorYPos };

			if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
				object.transform.rotation += mMouseSensitivity * rotate * deltaTime;
		}

		object.transform.rotation.x = glm::clamp(object.transform.rotation.x, -1.5f, 1.5f);
		object.transform.rotation.y = glm::mod(object.transform.rotation.y, glm::two_pi<float>());
	}

	void KeyboardMovementController::applyMovement(GLFWwindow* window, float deltaTime, VreSceneObject& object)
	{
		// Key input movement
		float yaw = object.transform.rotation.y;
		float pitch = object.transform.rotation.x;
		const glm::vec3 forwardDir{sin(yaw), 0.0f, cos(yaw)};
		const glm::vec3 rightDir{forwardDir.z, 0.0f, -forwardDir.x};
		const glm::vec3 upDir{0.0f, -1.0f, 0.0f};
		// Todo: update all directions properly according to the rotation 
		// maybe use rotation matrix see: https://en.wikipedia.org/wiki/Rotation_matrix

		glm::vec3 moveDir{0.0f};
		if (glfwGetKey(window, mKeys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
		if (glfwGetKey(window, mKeys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
		if (glfwGetKey(window, mKeys.moveRight) == GLFW_PRESS) moveDir += rightDir;
		if (glfwGetKey(window, mKeys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
		if (glfwGetKey(window, mKeys.moveUp) == GLFW_PRESS) moveDir += upDir;
		if (glfwGetKey(window, mKeys.moveDown) == GLFW_PRESS) moveDir -= upDir;

		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
			object.transform.location += mMoveSpeed * glm::normalize(moveDir) * deltaTime;

		// Mouse pan input movement
		toogleMousePan(window, glfwGetMouseButton(window, mKeys.mousePan) == GLFW_PRESS);
		if (mMousePanEnabled)
		{
			double cursorXPos, cursorYPos;
			glfwGetCursorPos(window, &cursorXPos, &cursorYPos);

			moveDir -= rightDir * (static_cast<float>(cursorXPos) - mPreviousCursorPos.x);
			moveDir += upDir * (static_cast<float>(cursorYPos) - mPreviousCursorPos.y);
			mPreviousCursorPos = { cursorXPos, cursorYPos };

			if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
				object.transform.location += mMouseSensitivity * glm::normalize(moveDir) * deltaTime;
		}
	}

	void KeyboardMovementController::toggleMouseRotate(GLFWwindow* window, bool enabled)
	{
		if (mMousePanEnabled)
			return;

		if (!mMouseRotateEnabled && enabled)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			mMouseRotateEnabled = true;

			double cursorXPos, cursorYPos;
			glfwGetCursorPos(window, &cursorXPos, &cursorYPos);
			mPreviousCursorPos = { static_cast<float>(cursorXPos), static_cast<float>(cursorYPos) };
		}
		else if (mMouseRotateEnabled && !enabled)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			mMouseRotateEnabled = false;
		}
	}

	void KeyboardMovementController::toogleMousePan(GLFWwindow* window, bool enabled)
	{
		if (mMouseRotateEnabled)
			return;

		if (!mMousePanEnabled && enabled)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			mMousePanEnabled = true;

			double cursorXPos, cursorYPos;
			glfwGetCursorPos(window, &cursorXPos, &cursorYPos);
			mPreviousCursorPos = { static_cast<float>(cursorXPos), static_cast<float>(cursorYPos) };
		}
		else if (mMousePanEnabled && !enabled)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			mMousePanEnabled = false;
		}
	}

} // namespace vre