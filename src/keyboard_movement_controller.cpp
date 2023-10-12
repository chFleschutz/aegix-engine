#include "keyboard_movement_controller.h"

#include "input.h"

#include "glm/gtc/constants.hpp"

#include <limits>

namespace vre
{
	void KeyboardMovementController::begin()
	{
		mPreviousCursorPos = Input::instance().cursorPosition();
	}

	void KeyboardMovementController::update(float deltaSeconds)
	{
		applyRotation(deltaSeconds);
		applyMovement(deltaSeconds);
	}

	void KeyboardMovementController::applyRotation(float deltaSeconds)
	{
		auto& transform = getComponent<TransformComponent>();

		// Key input rotation
		glm::vec3 rotate{0.0f};
		if (Input::instance().keyPressed(mKeys.lookRight)) rotate.y += 1.0f;
		if (Input::instance().keyPressed(mKeys.lookLeft)) rotate.y -= 1.0f;
		if (Input::instance().keyPressed(mKeys.lookUp)) rotate.x += 1.0f;
		if (Input::instance().keyPressed(mKeys.lookDown)) rotate.x -= 1.0f;

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
		{
			transform.Rotation += mLookSpeed * glm::normalize(rotate) * deltaSeconds;
		}

		// Mouse input rotation
		toggleMouseRotate(Input::instance().mouseButtonPressed(mKeys.mouseRotate));
		if (mMouseRotateEnabled)
		{
			auto cursorPos = Input::instance().cursorPosition();
			rotate.x -= cursorPos.y - mPreviousCursorPos.y;
			rotate.y += cursorPos.x - mPreviousCursorPos.x;
			mPreviousCursorPos = cursorPos;

			if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
			{
				transform.Rotation += mMouseSensitivity * rotate * deltaSeconds;
			}
		}

		transform.Rotation.x = glm::clamp(transform.Rotation.x, -1.5f, 1.5f);
		transform.Rotation.y = glm::mod(transform.Rotation.y, glm::two_pi<float>());
	}

	void KeyboardMovementController::applyMovement(float deltaSeconds)
	{
		auto& transform = getComponent<TransformComponent>();

		// Key input movement
		float yaw = transform.Rotation.y;
		float pitch = transform.Rotation.x;

		const glm::vec3 forwardDir{sin(yaw), 0.0f, cos(yaw)};
		const glm::vec3 rightDir{forwardDir.z, 0.0f, -forwardDir.x};
		const glm::vec3 upDir{0.0f, -1.0f, 0.0f};
		// Todo: update all directions properly according to the rotation 
		// maybe use rotation matrix see: https://en.wikipedia.org/wiki/Rotation_matrix

		glm::vec3 moveDir{0.0f};
		if (Input::instance().keyPressed(mKeys.moveForward)) moveDir += forwardDir;
		if (Input::instance().keyPressed(mKeys.moveBackward)) moveDir -= forwardDir;
		if (Input::instance().keyPressed(mKeys.moveRight)) moveDir += rightDir;
		if (Input::instance().keyPressed(mKeys.moveLeft)) moveDir -= rightDir;
		if (Input::instance().keyPressed(mKeys.moveUp)) moveDir += upDir;
		if (Input::instance().keyPressed(mKeys.moveDown)) moveDir -= upDir;

		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
		{ 
			transform.Location += mMoveSpeed * glm::normalize(moveDir) * deltaSeconds;
		}

		// Mouse pan input movement
		toogleMousePan(Input::instance().mouseButtonPressed(mKeys.mousePan));
		if (mMousePanEnabled)
		{
			auto cursorPos = Input::instance().cursorPosition();
			moveDir -= rightDir * (cursorPos.x - mPreviousCursorPos.x);
			moveDir += upDir * (cursorPos.y - mPreviousCursorPos.y);
			mPreviousCursorPos = cursorPos;

			if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
			{
				transform.Location += mMouseSensitivity * glm::normalize(moveDir) * deltaSeconds;
			}
		}
	}

	void KeyboardMovementController::toggleMouseRotate(bool enabled)
	{
		if (mMousePanEnabled)
			return;

		if (!mMouseRotateEnabled && enabled)
		{
			Input::instance().setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			mMouseRotateEnabled = true;
			mPreviousCursorPos = Input::instance().cursorPosition();
		}
		else if (mMouseRotateEnabled && !enabled)
		{
			Input::instance().setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			mMouseRotateEnabled = false;
		}
	}

	void KeyboardMovementController::toogleMousePan(bool enabled)
	{
		if (mMouseRotateEnabled)
			return;

		if (!mMousePanEnabled && enabled)
		{
			Input::instance().setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			mMousePanEnabled = true;
			mPreviousCursorPos = Input::instance().cursorPosition();
		}
		else if (mMousePanEnabled && !enabled)
		{
			Input::instance().setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			mMousePanEnabled = false;
		}
	}

} // namespace vre