#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"

namespace Aegix::AI
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

		virtual Aegix::Physics::Force computeForce() override
		{
			if (!m_target.has_value())
				return Aegix::Physics::Force{};

			auto& transform = m_aiComponent->getComponent<Aegix::Component::Transform>();
			auto& playerTransform = m_target.value().entity.getComponent<Aegix::Component::Transform>();

			Aegix::Physics::Force force{};
			force.linear = Aegix::MathLib::normalize(transform.location - playerTransform.location) * m_limits.maxLinearForce;
			return force;
		}

	private:
		std::optional<EntityKnowledge> m_target;
	};
}