#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"

namespace VEAI
{
	class SteeringBehaviourFlee : public SteeringBehaviour
	{
	public:
		void setTarget(const EntityKnowledge& target)
		{
			m_target = target;
		}

	protected:
		virtual VEPhysics::Force computeForce() override
		{
			if (!m_target.has_value())
				return VEPhysics::Force{};

			auto& transform = m_aiComponent->getComponent<VEComponent::Transform>();
			auto& playerTransform = m_target.value().entity.getComponent<VEComponent::Transform>();

			VEPhysics::Force force{};
			force.linear = glm::normalize(transform.location - playerTransform.location) * m_limits.maxLinearForce;
			return force;
		}

	private:
		std::optional<EntityKnowledge> m_target;
	};
}