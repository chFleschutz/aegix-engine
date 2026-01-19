#include "pch.h"
#include "dynamic_movement_controller.h"

#include "core/input.h"
#include "physics/motion_dynamics.h"

namespace Aegis::Scripting
{
	void DynamicMovementController::update(float deltaSeconds)
	{
		auto& transform = get<Transform>();
		auto& dynamics = get<Physics::MotionDynamics>();

		bool forwardPressed = Input::instance().keyPressed(m_keys.moveForward);
		bool backwardPressed = Input::instance().keyPressed(m_keys.moveBackward);
		bool leftPressed = Input::instance().keyPressed(m_keys.rotateLeft);
		bool rightPressed = Input::instance().keyPressed(m_keys.rotateRight);

		auto speed = dynamics.linearSpeed();
		auto angularSpeed = dynamics.angularSpeed();

		// Directional forces
		glm::vec3 move{ 0.0f };
		if (forwardPressed)
			move += transform.forward() * m_linearforce;
		if (backwardPressed)
			move -= transform.forward() * m_linearforce;
		if (!forwardPressed and !backwardPressed and speed > 0.0f)
			move = -dynamics.moveDirection() * m_linearforce * speed;

		// Angular forces
		glm::vec3 rotation{ 0.0f };
		if (leftPressed)
			rotation += transform.up() * m_angularforce;
		if (rightPressed)
			rotation -= transform.up() * m_angularforce;
		if (!leftPressed and !rightPressed and angularSpeed > 0.0f)
			rotation = -dynamics.angularDirection() * m_angularforce * angularSpeed;

		// Add forces
		dynamics.addLinearForce(move);
		dynamics.addAngularForce(rotation);
	}
}
