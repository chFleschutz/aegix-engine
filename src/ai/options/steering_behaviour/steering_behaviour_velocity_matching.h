#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"
#include "utils/math_utils.h"
#include "physics/motion_dynamics.h"

#include <vector>

namespace VEAI
{
	class SteeringBehaviourVelocityMatching : public SteeringBehaviour
	{
	public:
		void setEnitites(const std::vector<VEScene::Entity>& entities)
		{
			m_entities = entities;
		}
		
	protected:
		virtual void computeForce() override
		{
			if (m_entities.empty())
				return;

			auto& thisTransform = m_aiComponent->getComponent<VEComponent::Transform>();

			Vector3 averageVelocity{ 0.0f };
			for (const auto& entity : m_entities)
			{
				// TODO: check if entity is the same as this entity

				auto& otherTransform = entity.getComponent<VEComponent::Transform>();
				auto& dynamics = entity.getComponent<VEPhysics::MotionDynamics>();

				auto direction = otherTransform.location - thisTransform.location;
				auto distance = direction.length();

				if (distance < m_activationRadius)
					averageVelocity += dynamics.velocity;
			}

			return SteeringForce{ averageVelocity / m_entities.size(), 0.0f};
		}

	private:
		std::vector<VEScene::Entity> m_entities;
		float m_activationRadius = 1.0f;
	};
}
