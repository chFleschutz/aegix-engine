#include "kinematic_movement_controller.h"

#include "core/input.h"


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
		auto& transform = component<Aegix::Component::Transform>();

		// Key input rotation
		glm::vec3 rotate{0.0f};
		if (Input::instance().keyPressed(m_keys.lookRight)) rotate.z -= 1.0f;
		if (Input::instance().keyPressed(m_keys.lookLeft)) rotate.z += 1.0f;
		if (Input::instance().keyPressed(m_keys.lookUp)) rotate.x += 1.0f;
		if (Input::instance().keyPressed(m_keys.lookDown)) rotate.x -= 1.0f;

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
		{
			transform.rotation += m_lookSpeed * Aegix::MathLib::normalize(rotate) * deltaSeconds;
		}

		// Mouse input rotation
		toggleMouseRotate(Input::instance().mouseButtonPressed(m_keys.mouseRotate));
		if (m_mouseRotateEnabled)
		{
			auto cursorPos = Input::instance().cursorPosition();
			rotate.x -= cursorPos.y - m_previousCursorPos.y;
			rotate.z -= cursorPos.x - m_previousCursorPos.x;
			m_previousCursorPos = cursorPos;

			if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
			{
				transform.rotation += m_mouseSensitivity * rotate * deltaSeconds;
			}
		}

		transform.rotation.x = glm::clamp(transform.rotation.x, -1.5f, 1.5f);
		transform.rotation.z = glm::mod(transform.rotation.z, glm::two_pi<float>());
	}

	void KinematcMovementController::applyMovement(float deltaSeconds)
	{
		auto& transform = component<Aegix::Component::Transform>();
		auto forwardDir = transform.forward();
		auto rightDir = transform.right();
		auto upDir = MathLib::UP;

		// Key input movement
		glm::vec3 moveDir{0.0f};
		if (Input::instance().keyPressed(m_keys.moveForward)) moveDir += forwardDir;
		if (Input::instance().keyPressed(m_keys.moveBackward)) moveDir -= forwardDir;
		if (Input::instance().keyPressed(m_keys.moveRight)) moveDir += rightDir;
		if (Input::instance().keyPressed(m_keys.moveLeft)) moveDir -= rightDir;
		if (Input::instance().keyPressed(m_keys.moveUp)) moveDir += upDir;
		if (Input::instance().keyPressed(m_keys.moveDown)) moveDir -= upDir;

		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
		{ 
			transform.location += m_moveSpeed * Aegix::MathLib::normalize(moveDir) * deltaSeconds;
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
				transform.location += m_mouseSensitivity * Aegix::MathLib::normalize(moveDir) * deltaSeconds;
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
