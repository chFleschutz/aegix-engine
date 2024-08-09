#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"

namespace Aegix::AI
{
	class SteeringBehaviourSeek : public SteeringBehaviour
	{
	public:
		explicit SteeringBehaviourSeek(AIComponent* aiComponent, const EntityKnowledge& target)
			: SteeringBehaviour(aiComponent), m_target(target) {}

		virtual Aegix::Physics::Force computeForce() override
		{
			auto& transform = m_aiComponent->getComponent<Aegix::Component::Transform>();
			auto& playerTransform = m_target.entity.getComponent<Aegix::Component::Transform>();

			Aegix::Physics::Force force{};
			force.linear = Aegix::MathLib::normalize(playerTransform.location - transform.location) * m_limits.maxLinearForce;
			return force;
		}

	private:
		EntityKnowledge m_target;
	};
}
