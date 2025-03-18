#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"
#include "scene/components.h"

#include <optional>

namespace Aegix::AI
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

		virtual Aegix::Physics::Force computeForce() override
		{
			if (!m_target.has_value())
				return Aegix::Physics::Force{};

			auto& transform = m_aiComponent->getComponent<Aegix::Component::Transform>();
			auto& playerTransform = m_target.value().entity.getComponent<Aegix::Component::Transform>();
			auto& dynamics = m_aiComponent->getComponent<Aegix::Physics::MotionDynamics>();

			const auto playerDirection = playerTransform.location - transform.location;
			const auto distance = glm::length(playerDirection);

			// Check if close enough
			if (distance < m_haltDistance)
			{
				dynamics.haltMotion();
				stop();
				return Aegix::Physics::Force{};
			}

			const auto maxSpeed = dynamics.properties().maxLinearSpeed;
			const auto brakeDistance = (maxSpeed * maxSpeed * dynamics.properties().mass) / (2.0f * m_limits.maxLinearForce);
			auto desiredSpeed = maxSpeed;
			
			// Slow down if close enough
			if (distance < brakeDistance)
				desiredSpeed *= distance / brakeDistance;

			// Compute force
			Aegix::Physics::Force force{};
			const auto desiredVelocity = Aegix::MathLib::normalize(playerDirection) * desiredSpeed;
			force.linear = Aegix::MathLib::normalize(desiredVelocity - dynamics.linearVelocity()) * m_limits.maxLinearForce;

			return force;
		}

	private:
		// Distance at which the option will stop
		float m_haltDistance = 0.1f;

		std::optional<EntityKnowledge> m_target;
	};
}
