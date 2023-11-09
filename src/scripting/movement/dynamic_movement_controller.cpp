#include "dynamic_movement_controller.h"

#include "core/input.h"
#include "physics/motion_dynamics.h"

#include <iostream>

namespace VEScripting
{
	void DynamicMovementController::update(float deltaSeconds)
	{
		auto& transform = getComponent<VEComponent::Transform>();
		auto& dynamics = getComponent<VEPhysics::MotionDynamics>();

		bool forwardPressed = Input::instance().keyPressed(m_keys.moveForward);
		bool backwardPressed = Input::instance().keyPressed(m_keys.moveBackward);
		bool leftPressed = Input::instance().keyPressed(m_keys.rotateLeft);
		bool rightPressed = Input::instance().keyPressed(m_keys.rotateRight);

		auto speed = dynamics.speed();
		auto angularSpeed = dynamics.angularSpeed();

		// Directional forces
		Vector3 moveForce{ 0.0f };
		if (forwardPressed)
			moveForce += transform.forward() * m_acceleration;
		if (backwardPressed)
			moveForce -= transform.forward() * m_acceleration;
		if (!forwardPressed and !backwardPressed and speed > 0.0f)
			moveForce = -dynamics.moveDirection() * m_dragFactor * speed;

		// Angular forces
		Vector3 rotation{ 0.0f };
		if (leftPressed)
			rotation += transform.up() * m_acceleration;
		if (rightPressed)
			rotation -= transform.up() * m_acceleration;
		if (!leftPressed and !rightPressed and angularSpeed > 0.0f)
			rotation = -dynamics.angularVelocity() * m_dragFactor * angularSpeed;

		// Add forces
		dynamics.addForce(moveForce);
		dynamics.addAngularForce(rotation);
	}

} // namespace VEScripting
