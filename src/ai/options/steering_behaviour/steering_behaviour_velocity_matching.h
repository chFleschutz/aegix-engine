#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"
#include "utils/math_utils.h"
#include "physics/motion_dynamics.h"

#include <vector>

namespace Aegix::AI
{
	class SteeringBehaviourVelocityMatching : public SteeringBehaviour
	{
	public:
		SteeringBehaviourVelocityMatching(AIComponent* aiComponent, const EntityGroupKnowledge& group)
			: SteeringBehaviour(aiComponent), m_group(group) {}

		virtual Aegix::Physics::Force computeForce() override
		{
			if (m_group.entities.empty())
				return Aegix::Physics::Force{};

			auto& thisTransform = m_aiComponent->getComponent<Aegix::Component::Transform>();

			Vector3 averageVelocity{ 0.0f };
			for (const auto& entity : m_group.entities)
			{
				if (entity == m_aiComponent->entity())
					continue;

				auto& otherTransform = entity.getComponent<Aegix::Component::Transform>();
				auto& dynamics = entity.getComponent<Aegix::Physics::MotionDynamics>();

				auto direction = otherTransform.location - thisTransform.location;
				auto distance = glm::length(direction);

				if (distance < m_activationRadius)
					averageVelocity += dynamics.linearVelocity();
			}

			Aegix::Physics::Force force{};
			force.linear = averageVelocity / static_cast<float>(m_group.entities.size());
			return force;
		}

	private:
		EntityGroupKnowledge m_group;

		float m_activationRadius = 1.0f;
	};
}
