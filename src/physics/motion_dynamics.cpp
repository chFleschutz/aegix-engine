#include "motion_dynamics.h"

#include "scene/components.h"

namespace VEPhysics
{
	void MotionDynamics::addForce(const Vector3& force)
	{
		m_accumulatedForce += force;
	}

	void MotionDynamics::addAngularForce(const Vector3& angularForce)
	{
		m_accumulatedAngularForce += angularForce;
	}

	void MotionDynamics::update(float deltaSeconds)
	{
		applyForces(deltaSeconds);

		// Update transform
		auto& transform = getComponent<VEComponent::Transform>();
		transform.location += m_velocity * deltaSeconds;
		transform.rotation += m_angularVelocity * deltaSeconds;
	}

	void MotionDynamics::applyForces(float deltaSeconds)
	{
		// Calculate new velocities
		m_velocity += m_accumulatedForce / m_properties.mass * deltaSeconds;
		m_angularVelocity += m_accumulatedAngularForce / m_properties.mass * deltaSeconds;
		
		// Limit velocities
		m_velocity *= std::min(1.0f, m_properties.maxLinearSpeed / speed());
		m_angularVelocity *= std::min(1.0f, m_properties.maxAngularSpeed / angularSpeed());

		// Reset accumulated forces
		m_accumulatedForce = Vector3{ 0.0f };
		m_accumulatedAngularForce = Vector3{ 0.0f };
	}

} // namespace VEPhysics
