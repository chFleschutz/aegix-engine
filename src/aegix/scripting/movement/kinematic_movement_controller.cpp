#include "pch.h"

#include "kinematic_movement_controller.h"

#include "core/input.h"
#include "math/math.h"

namespace Aegix::Scripting
{
	void KinematcMovementController::begin()
	{
		m_previousCursorPos = Input::instance().cursorPosition();
	}

	void KinematcMovementController::update(float deltaSeconds)
	{
		applyRotation(deltaSeconds);
		applyMovement(deltaSeconds);
	}

	void KinematcMovementController::applyRotation(float deltaSeconds)
	{
		auto& transform = get<Transform>();

		// Key input rotation
		glm::vec3 rotate{ 0.0f };
		if (Input::instance().keyPressed(m_keys.lookRight)) rotate.z -= m_lookSpeed;
		if (Input::instance().keyPressed(m_keys.lookLeft)) rotate.z += m_lookSpeed;
		if (Input::instance().keyPressed(m_keys.lookUp)) rotate.x += m_lookSpeed;
		if (Input::instance().keyPressed(m_keys.lookDown)) rotate.x -= m_lookSpeed;

		// Mouse input rotation
		toggleMouseRotate(Input::instance().mouseButtonPressed(m_keys.mouseRotate));
		if (m_mouseRotateEnabled)
		{
			auto cursorPos = Input::instance().cursorPosition();
			rotate.x -= (cursorPos.y - m_previousCursorPos.y) * m_mouseSensitivity;
			rotate.z -= (cursorPos.x - m_previousCursorPos.x) * m_mouseSensitivity;
			m_previousCursorPos = cursorPos;
		}

		if (glm::length(rotate) > std::numeric_limits<float>::epsilon())
		{
			rotate *= deltaSeconds;

			glm::vec3 rotation = glm::eulerAngles(transform.rotation);
			rotation.x = glm::clamp(rotation.x + rotate.x, -1.5f, 1.5f);
			rotation.z = glm::mod(rotation.z + rotate.z, glm::two_pi<float>());
			rotation.y = 0.0f; // Lock rotation around y-axis
			transform.rotation = glm::quat(rotation);
		}
	}

	void KinematcMovementController::applyMovement(float deltaSeconds)
	{
		auto& transform = get<Transform>();
		auto forwardDir = transform.forward();
		auto rightDir = transform.right();
		auto upDir = Math::World::UP;

		// Key input movement
		glm::vec3 moveDir{ 0.0f };
		if (Input::instance().keyPressed(m_keys.moveForward)) moveDir += forwardDir;
		if (Input::instance().keyPressed(m_keys.moveBackward)) moveDir -= forwardDir;
		if (Input::instance().keyPressed(m_keys.moveRight)) moveDir += rightDir;
		if (Input::instance().keyPressed(m_keys.moveLeft)) moveDir -= rightDir;
		if (Input::instance().keyPressed(m_keys.moveUp)) moveDir += upDir;
		if (Input::instance().keyPressed(m_keys.moveDown)) moveDir -= upDir;

		if (glm::length(moveDir) > std::numeric_limits<float>::epsilon())
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

			if (glm::length(moveDir) > std::numeric_limits<float>::epsilon())
			{
				transform.location += moveDir * deltaSeconds;
			}
		}
	}

	void KinematcMovementController::toggleMouseRotate(bool enabled)
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

	void KinematcMovementController::toogleMousePan(bool enabled)
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
}
