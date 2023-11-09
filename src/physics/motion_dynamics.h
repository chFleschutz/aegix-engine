#pragma once

#include "scripting/script_base.h"
#include "utils/math_utils.h"

namespace VEPhysics
{
	class MotionDynamics : public VEScripting::ScriptBase
	{
	public:
		void addAcceleration(const Vector3& acceleration);
		void addAngularAcceleration(const Vector3& angularAcceleration);

		Vector3 velocity() const { return m_velocity; }
		Vector3 angularVelocity() const { return m_angularVelocity; }

		float speed() const { return glm::length(m_velocity); }
		float angularSpeed() const { return glm::length(m_angularVelocity); }

		Vector3 moveDirection() const { return glm::normalize(m_velocity); }

		void update(float deltaSeconds) override;

	private:
		void applyAcceleration(float deltaSeconds);

		Vector3 m_velocity{ 0.0f };
		Vector3 m_angularVelocity{ 0.0f };

		Vector3 m_acceleration{ 0.0f };
		Vector3 m_angularAcceleration{ 0.0f };
	};

} // namespace VEPhysics