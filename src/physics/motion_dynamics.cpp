#include "motion_dynamics.h"

#include "scene/components.h"

namespace VEPhysics
{
	Force& Force::operator+=(const Force& other)
	{
		linear += other.linear;
		angular += other.angular;
		return *this;
	}

	Force& Force::operator-=(const Force& other)
	{
		linear -= other.linear;
		angular -= other.angular;
		return *this;
	}

	Force& Force::operator*=(const Force& other)
	{
		linear *= other.linear;
		angular *= other.angular;
		return *this;
	}

	Force& Force::operator/=(const Force& other)
	{
		linear /= other.linear;
		angular /= other.angular;
		return *this;
	}

	Force& Force::operator*=(float scalar)
	{
		linear *= scalar;
		angular *= scalar;
		return *this;
	}

	Force& Force::operator/=(float scalar)
	{
		linear /= scalar;
		angular /= scalar;
		return *this;
	}


	void MotionDynamics::addLinearForce(const Vector3& force)
	{
		m_accumulatedLinearForce += force;
	}

	void MotionDynamics::addAngularForce(const Vector3& angularForce)
	{
		m_accumulatedAngularForce += angularForce;
	}

	void MotionDynamics::haltMotion()
	{
		m_linearVelocity = Vector3{ 0.0f };
		m_angularVelocity = Vector3{ 0.0f };
	}

	Vector3 MotionDynamics::moveDirection() const
	{
		assert(linearSpeed() > 0.0f && "Direction of zero-vector is undefined");
		return glm::normalize(m_linearVelocity);
	}

	Vector3 MotionDynamics::angularDirection() const
	{
		assert(angularSpeed() > 0.0f && "Direction of zero-vector is undefined");
		return glm::normalize(m_angularVelocity);
	}

	void MotionDynamics::update(float deltaSeconds)
	{
		addFriction(deltaSeconds);

		applyForces(deltaSeconds);
		// Update transform
		auto& transform = getComponent<VEComponent::Transform>();
		transform.location += m_linearVelocity * deltaSeconds;
		transform.rotation += m_angularVelocity * deltaSeconds;
	}

	void MotionDynamics::applyForces(float deltaSeconds)
	{
		// Calculate new velocities
		m_linearVelocity += m_accumulatedLinearForce / m_properties.mass * deltaSeconds;
		m_angularVelocity += m_accumulatedAngularForce / m_properties.mass * deltaSeconds;
		
		// Limit velocities
		m_linearVelocity *= std::min(1.0f, m_properties.maxLinearSpeed / linearSpeed());
		m_angularVelocity *= std::min(1.0f, m_properties.maxAngularSpeed / angularSpeed());

		// Reset accumulated forces
		m_accumulatedLinearForce = Vector3{ 0.0f };
		m_accumulatedAngularForce = Vector3{ 0.0f };
	}

	void MotionDynamics::addFriction(float deltaSeconds)
	{
		addLinearForce(-m_linearVelocity * m_properties.linearFriction);
		addAngularForce(-m_angularVelocity * m_properties.angularFriction);
	}



} // namespace VEPhysics
