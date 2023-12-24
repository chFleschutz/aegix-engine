#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"

namespace VEAI
{
	class SteeringBehaviourSeek : public SteeringBehaviour
	{
	public:
		explicit SteeringBehaviourSeek(AIComponent* aiComponent, const EntityKnowledge& target)
			: SteeringBehaviour(aiComponent), m_target(target) {}

		virtual VEPhysics::Force computeForce() override
		{
			auto& transform = m_aiComponent->getComponent<VEComponent::Transform>();
			auto& playerTransform = m_target.entity.getComponent<VEComponent::Transform>();

			VEPhysics::Force force{};
			force.linear = MathLib::normalize(playerTransform.location - transform.location) * m_limits.maxLinearForce;
			return force;
		}

	private:
		EntityKnowledge m_target;
	};

} // namespace VEAI