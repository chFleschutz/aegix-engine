#pragma once

#include "ai/ai_component.h"
#include "ai/options/option.h"
#include "physics/motion_dynamics.h"
#include "utils/math_utils.h"

namespace VEAI
{
	class SteeringBehaviour : public Option
	{
	protected:
		virtual void updateOption(float deltaSeconds) override
		{
			auto& dynamics = m_aiComponent->getComponent<VEPhysics::MotionDynamics>();
			dynamics.addForce(computeForce());
			dynamics.addAngularForce(computeAngularForce());
		}

		virtual Vector3 computeForce() = 0;
		virtual Vector3 computeAngularForce() = 0;
	};

} // namespace VEAI