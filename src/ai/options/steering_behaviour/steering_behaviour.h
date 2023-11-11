#pragma once

#include "ai/ai_component.h"
#include "ai/options/option.h"
#include "physics/motion_dynamics.h"
#include "scene/components.h"
#include "utils/math_utils.h"

#include <iostream>

namespace VEAI
{
	class SteeringBehaviour : public Option
	{
	public:
		struct Limits
		{
			float maxLinearForce = 20.0f;
			float maxAngularForce = 20.0f;
		};

		struct SteeringForce
		{
			Vector3 linear = Vector3{ 0.0f };
			Vector3 angular = Vector3{ 0.0f };
		};

		void setLimits(const Limits& limits) { m_limits = limits; }

	protected:
		virtual void updateOption(float deltaSeconds) override
		{
			assert(m_aiComponent->hasComponent<VEPhysics::MotionDynamics>() && "SteeringBehaviour needs MotionDynamics component");
			
			auto& dynamics = m_aiComponent->getComponent<VEPhysics::MotionDynamics>();
			auto& name = m_aiComponent->getComponent<VEComponent::Name>().name;

			auto force = computeForce();
			dynamics.addForce(force.linear);
			dynamics.addAngularForce(force.angular);
		}

		virtual SteeringForce computeForce() = 0;

		Limits m_limits;
	};

} // namespace VEAI