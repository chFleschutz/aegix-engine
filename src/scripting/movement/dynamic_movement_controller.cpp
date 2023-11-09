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

		Vector3 forwardDir = transform.forward();
		Vector3 rightDir = transform.right();
		Vector3 upDir = transform.up();

		Vector3 move{ 0.0f };
		if (Input::instance().keyPressed(m_keys.moveForward)) move += forwardDir * m_acceleration;
		if (Input::instance().keyPressed(m_keys.moveBackward)) move -= forwardDir * m_acceleration;
		dynamics.addAcceleration(move);
		
		Vector3 rotation{ 0.0f };
		if (Input::instance().keyPressed(m_keys.rotateRight)) rotation -= upDir * m_acceleration;
		if (Input::instance().keyPressed(m_keys.rotateLeft)) rotation += upDir * m_acceleration;
		dynamics.addAngularAcceleration(rotation);
	}

} // namespace VEScripting