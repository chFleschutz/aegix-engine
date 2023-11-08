#include "motion_dynamics.h"

#include "scene/components.h"

namespace VEPhysics
{
	void MotionDynamics::addAcceleration(const Vector3& acceleration)
	{
		m_acceleration += acceleration;
	}

	void MotionDynamics::addAngularAcceleration(const Vector3& angularAcceleration)
	{
		m_angularAcceleration += angularAcceleration;
	}

	void MotionDynamics::update(float deltaSeconds)
	{
		applyAcceleration(deltaSeconds);

		// Update transform
		auto& transform = getComponent<vre::TransformComponent>();
		transform.location += m_velocity * deltaSeconds;
		transform.rotation += m_angularVelocity * deltaSeconds;
	}

	void MotionDynamics::applyAcceleration(float deltaSeconds)
	{
		// Update velocities
		m_velocity += m_acceleration * deltaSeconds;
		m_angularVelocity += m_angularAcceleration * deltaSeconds;
		
		// Reset acceleration
		m_acceleration = Vector3::zero();
		m_angularAcceleration = Vector3::zero();
	}
}
