#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"

namespace Aegis::AI
{
	class SteeringBehaviourFlee : public SteeringBehaviour
	{
	public:
		explicit SteeringBehaviourFlee(AIComponent* aiComponent)
			: SteeringBehaviour(aiComponent) {}

		void setTarget(const EntityKnowledge& target)
		{
			m_target = target;
		}

		virtual Aegis::Physics::Force computeForce() override
		{
			if (!m_target.has_value())
				return Aegis::Physics::Force{};

			auto& transform = m_aiComponent->getComponent<Aegis::Component::Transform>();
			auto& playerTransform = m_target.value().entity.getComponent<Aegis::Component::Transform>();

			Aegis::Physics::Force force{};
			force.linear = Aegis::MathLib::normalize(transform.location - playerTransform.location) * m_limits.maxLinearForce;
			return force;
		}

	private:
		std::optional<EntityKnowledge> m_target;
	};
}