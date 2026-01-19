#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"
#include "scene/components.h"

#include <optional>

namespace Aegis::AI
{
	class SteeringBehaviourArrive : public SteeringBehaviour
	{
	public:
		// TODO: Remove this constructor
		explicit SteeringBehaviourArrive(AIComponent* aiComponent)
			: SteeringBehaviour(aiComponent) {}

		SteeringBehaviourArrive(AIComponent* aiComponent, const EntityKnowledge& target)
			: SteeringBehaviour(aiComponent), m_target(target) {}

		void setTarget(const EntityKnowledge& target) { m_target = target; }

		virtual Aegis::Physics::Force computeForce() override
		{
			if (!m_target.has_value())
				return Aegis::Physics::Force{};

			auto& transform = m_aiComponent->getComponent<Aegis::Component::Transform>();
			auto& playerTransform = m_target.value().entity.getComponent<Aegis::Component::Transform>();
			auto& dynamics = m_aiComponent->getComponent<Aegis::Physics::MotionDynamics>();

			const auto playerDirection = playerTransform.location - transform.location;
			const auto distance = glm::length(playerDirection);

			// Check if close enough
			if (distance < m_haltDistance)
			{
				dynamics.haltMotion();
				stop();
				return Aegis::Physics::Force{};
			}

			const auto maxSpeed = dynamics.properties().maxLinearSpeed;
			const auto brakeDistance = (maxSpeed * maxSpeed * dynamics.properties().mass) / (2.0f * m_limits.maxLinearForce);
			auto desiredSpeed = maxSpeed;
			
			// Slow down if close enough
			if (distance < brakeDistance)
				desiredSpeed *= distance / brakeDistance;

			// Compute force
			Aegis::Physics::Force force{};
			const auto desiredVelocity = Aegis::MathLib::normalize(playerDirection) * desiredSpeed;
			force.linear = Aegis::MathLib::normalize(desiredVelocity - dynamics.linearVelocity()) * m_limits.maxLinearForce;

			return force;
		}

	private:
		// Distance at which the option will stop
		float m_haltDistance = 0.1f;

		std::optional<EntityKnowledge> m_target;
	};
}
