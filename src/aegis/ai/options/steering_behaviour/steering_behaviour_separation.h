#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"
#include "utils/math_utils.h"

namespace Aegis::AI
{
	class SteeringBehaviourSeparation : public SteeringBehaviour
	{
	public:
		SteeringBehaviourSeparation(AIComponent* aiComponent, const EntityGroupKnowledge& group)
			: SteeringBehaviour(aiComponent), m_group(group) {}

		virtual Aegis::Physics::Force computeForce() override
		{
			if (m_group.entities.empty())
				return Aegis::Physics::Force{};

			auto& tranform = m_aiComponent->getComponent<Aegis::Component::Transform>();
			auto& dynamics = m_aiComponent->getComponent<Aegis::Physics::MotionDynamics>();

			Aegis::Physics::Force force{};
			for (const auto& entity : m_group.entities)
			{
				auto& otherTransform = entity.getComponent<Aegis::Component::Transform>();

				auto OtherDirection = otherTransform.location - tranform.location;
				auto distance = glm::length(OtherDirection);

				if (distance == 0.0f)
					continue;

				if (distance < m_activationRadius and Aegis::MathLib::inFOV(dynamics.linearVelocity(), OtherDirection, m_fov))
				{
					auto strength = m_limits.maxLinearForce * (m_activationRadius - distance) / m_activationRadius;
					force.linear += Aegis::MathLib::normalize(-OtherDirection) * strength;
				}
			}

			return force;
		}

	private:
		float m_activationRadius = 5.0f;
		float m_fov = glm::radians(360.0f);

		EntityGroupKnowledge m_group;
	};
}
