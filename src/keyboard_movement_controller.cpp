#include "keyboard_movement_controller.h"

#include "core/input.h"

#include "glm/gtc/constants.hpp"

#include <limits>

namespace VEScripting
{
	void KeyboardMovementController::begin()
	{
		m_previousCursorPos = Input::instance().cursorPosition();
	}

	void KeyboardMovementController::update(float deltaSeconds)
	{
		applyRotation(deltaSeconds);
		applyMovement(deltaSeconds);
	}

	void KeyboardMovementController::applyRotation(float deltaSeconds)
	{
		auto& transform = getComponent<VEComponent::Transform>();

		// Key input rotation
		Vector3 rotate{0.0f};
		if (Input::instance().keyPressed(m_keys.lookRight)) rotate.y += 1.0f;
		if (Input::instance().keyPressed(m_keys.lookLeft)) rotate.y -= 1.0f;
		if (Input::instance().keyPressed(m_keys.lookUp)) rotate.x += 1.0f;
		if (Input::instance().keyPressed(m_keys.lookDown)) rotate.x -= 1.0f;

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
		{
			transform.rotation += m_lookSpeed * glm::normalize(rotate) * deltaSeconds;
		}

		// Mouse input rotation
		toggleMouseRotate(Input::instance().mouseButtonPressed(m_keys.mouseRotate));
		if (m_mouseRotateEnabled)
		{
			auto cursorPos = Input::instance().cursorPosition();
			rotate.x -= cursorPos.y - m_previousCursorPos.y;
			rotate.y += cursorPos.x - m_previousCursorPos.x;
			m_previousCursorPos = cursorPos;

			if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
			{
				transform.rotation += m_mouseSensitivity * rotate * deltaSeconds;
			}
		}

		transform.rotation.x = glm::clamp(transform.rotation.x, -1.5f, 1.5f);
		transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());
	}

	void KeyboardMovementController::applyMovement(float deltaSeconds)
	{
		auto& transform = getComponent<VEComponent::Transform>();

		// Key input movement
		float yaw = transform.rotation.y;
		float pitch = transform.rotation.x;

		const Vector3 forwardDir{sin(yaw), 0.0f, cos(yaw)};
		const Vector3 rightDir{forwardDir.z, 0.0f, -forwardDir.x};
		const Vector3 upDir{0.0f, -1.0f, 0.0f};
		// Todo: update all directions properly according to the rotation 
		// maybe use rotation matrix see: https://en.wikipedia.org/wiki/Rotation_matrix

		Vector3 moveDir{0.0f};
		if (Input::instance().keyPressed(m_keys.moveForward)) moveDir += forwardDir;
		if (Input::instance().keyPressed(m_keys.moveBackward)) moveDir -= forwardDir;
		if (Input::instance().keyPressed(m_keys.moveRight)) moveDir += rightDir;
		if (Input::instance().keyPressed(m_keys.moveLeft)) moveDir -= rightDir;
		if (Input::instance().keyPressed(m_keys.moveUp)) moveDir += upDir;
		if (Input::instance().keyPressed(m_keys.moveDown)) moveDir -= upDir;

		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
		{ 
			transform.location += m_moveSpeed * glm::normalize(moveDir) * deltaSeconds;
		}

		// Mouse pan input movement
		toogleMousePan(Input::instance().mouseButtonPressed(m_keys.mousePan));
		if (m_mousePanEnabled)
		{
			auto cursorPos = Input::instance().cursorPosition();
			moveDir -= rightDir * (cursorPos.x - m_previousCursorPos.x);
			moveDir += upDir * (cursorPos.y - m_previousCursorPos.y);
			m_previousCursorPos = cursorPos;

			if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
			{
				transform.location += m_mouseSensitivity * glm::normalize(moveDir) * deltaSeconds;
			}
		}
	}

	void KeyboardMovementController::toggleMouseRotate(bool enabled)
	{
		if (m_mousePanEnabled)
			return;

		if (!m_mouseRotateEnabled && enabled)
		{
			Input::instance().setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			m_mouseRotateEnabled = true;
			m_previousCursorPos = Input::instance().cursorPosition();
		}
		else if (m_mouseRotateEnabled && !enabled)
		{
			Input::instance().setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			m_mouseRotateEnabled = false;
		}
	}

	void KeyboardMovementController::toogleMousePan(bool enabled)
	{
		if (m_mouseRotateEnabled)
			return;

		if (!m_mousePanEnabled && enabled)
		{
			Input::instance().setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			m_mousePanEnabled = true;
			m_previousCursorPos = Input::instance().cursorPosition();
		}
		else if (m_mousePanEnabled && !enabled)
		{
			Input::instance().setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			m_mousePanEnabled = false;
		}
	}

} // namespace VEScripting