#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"

namespace Aegis::AI
{
	class SteeringBehaviourSeek : public SteeringBehaviour
	{
	public:
		explicit SteeringBehaviourSeek(AIComponent* aiComponent, const EntityKnowledge& target)
			: SteeringBehaviour(aiComponent), m_target(target) {}

		virtual Aegis::Physics::Force computeForce() override
		{
			auto& transform = m_aiComponent->getComponent<Aegis::Component::Transform>();
			auto& playerTransform = m_target.entity.getComponent<Aegis::Component::Transform>();

			Aegis::Physics::Force force{};
			force.linear = Aegis::MathLib::normalize(playerTransform.location - transform.location) * m_limits.maxLinearForce;
			return force;
		}

	private:
		EntityKnowledge m_target;
	};
}
