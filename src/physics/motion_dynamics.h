#pragma once

#include "scene/script_base.h"
#include "utils/math_utils.h"

namespace VEPhysics
{
	class MotionDynamics : public VEScripting::ScriptBase
	{
	public:
		void addAcceleration(const Vector3& acceleration);
		void addAngularAcceleration(const Vector3& angularAcceleration);

	protected:
		void update(float deltaSeconds) override;

	private:
		void applyAcceleration(float deltaSeconds);

		Vector3 m_velocity;
		Vector3 m_angularVelocity;

		Vector3 m_acceleration;
		Vector3 m_angularAcceleration;
	};

} // namespace VEPhysics