#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"
#include "ai/knowledge.h"

namespace VEAI
{
	class SteeringBehaviourCohesion : public SteeringBehaviour
	{
	public:
		SteeringBehaviourCohesion(AIComponent* aiComponent, const EntityGroupKnowledge& group)
			: SteeringBehaviour(aiComponent), m_group(group) {}

		virtual VEPhysics::Force computeForce() override
		{
			if (m_group.entities.empty())
				return VEPhysics::Force{};

			auto& transform = m_aiComponent->getComponent<VEComponent::Transform>();

			Vector3 centerOfMass{ 0.0f };
			int relevantEntities = 0;
			for (const auto& entity : m_group.entities)
			{
				if (entity == m_aiComponent->entity())
					continue;

				auto& otherTransform = entity.getComponent<VEComponent::Transform>();

				auto direction = transform.location - otherTransform.location;
				float distance = glm::length(direction);

				if (distance < m_activationRadius)
				{
					centerOfMass += otherTransform.location;
					relevantEntities++;
				}
			}

			if (relevantEntities == 0)
				return VEPhysics::Force{};
				
			VEPhysics::Force force{};
			centerOfMass /= relevantEntities;
			auto dir = centerOfMass - transform.location;
			force.linear = MathLib::normalize(dir) * (glm::length(dir) / m_activationRadius);

			return force;
		}

	private:
		float m_activationRadius = 7.0f;

		EntityGroupKnowledge m_group;
	};

} // namespace VEAI