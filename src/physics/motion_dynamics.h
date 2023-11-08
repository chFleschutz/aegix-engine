#pragma once

#include "utils/math_utils.h"
#include "scene/script_component_base.h"

namespace VEPhysics
{
	class MotionDynamics : public vre::ScriptComponentBase
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
}