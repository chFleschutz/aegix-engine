#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"
#include "ai/knowledge.h"

namespace Aegix::AI
{
	class SteeringBehaviourCohesion : public SteeringBehaviour
	{
	public:
		SteeringBehaviourCohesion(AIComponent* aiComponent, const EntityGroupKnowledge& group)
			: SteeringBehaviour(aiComponent), m_group(group) {}

		virtual Aegix::Physics::Force computeForce() override
		{
			if (m_group.entities.empty())
				return Aegix::Physics::Force{};

			auto& transform = m_aiComponent->getComponent<Aegix::Component::Transform>();

			Vector3 centerOfMass{ 0.0f };
			int relevantEntities = 0;
			for (const auto& entity : m_group.entities)
			{
				if (entity == m_aiComponent->entity())
					continue;

				auto& otherTransform = entity.getComponent<Aegix::Component::Transform>();

				auto direction = transform.location - otherTransform.location;
				float distance = glm::length(direction);

				if (distance < m_activationRadius)
				{
					centerOfMass += otherTransform.location;
					relevantEntities++;
				}
			}

			if (relevantEntities == 0)
				return Aegix::Physics::Force{};
				
			Aegix::Physics::Force force{};
			centerOfMass /= relevantEntities;
			auto dir = centerOfMass - transform.location;
			force.linear = Aegix::MathLib::normalize(dir) * (glm::length(dir) / m_activationRadius);

			return force;
		}

	private:
		float m_activationRadius = 7.0f;

		EntityGroupKnowledge m_group;
	};
}
