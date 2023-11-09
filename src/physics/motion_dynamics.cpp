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
		// Apply directional forces
		m_velocity += m_accumulatedForce / m_mass * deltaSeconds;
		m_accumulatedForce = Vector3{ 0.0f };

		// Apply angular forces
		m_angularVelocity += m_accumulatedAngularForce / m_mass * deltaSeconds;
		m_accumulatedAngularForce = Vector3{ 0.0f };
	}

} // namespace VEPhysics
