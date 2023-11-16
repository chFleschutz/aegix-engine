#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"
#include "scene/components.h"

#include <optional>

namespace VEAI
{
	class SteeringBehaviourArrive : public SteeringBehaviour
	{
	public:
		explicit SteeringBehaviourArrive(AIComponent* aiComponent)
			: SteeringBehaviour(aiComponent) {}

		void setTarget(const EntityKnowledge& target) { m_target = target; }

		virtual VEPhysics::Force computeForce() override
		{
			if (!m_target.has_value())
				return VEPhysics::Force{};

			auto& transform = m_aiComponent->getComponent<VEComponent::Transform>();
			auto& playerTransform = m_target.value().entity.getComponent<VEComponent::Transform>();
			auto& dynamics = m_aiComponent->getComponent<VEPhysics::MotionDynamics>();

			const auto playerDirection = playerTransform.location - transform.location;
			const auto distance = glm::length(playerDirection);

			// Check if close enough
			if (distance < m_haltDistance)
			{
				dynamics.haltMotion();
				stop();
				return VEPhysics::Force{};
			}

			const auto maxSpeed = dynamics.properties().maxLinearSpeed;
			const auto brakeDistance = (maxSpeed * maxSpeed * dynamics.properties().mass) / (2.0f * m_limits.maxLinearForce);
			auto desiredSpeed = maxSpeed;
			
			// Slow down if close enough
			if (distance < brakeDistance)
				desiredSpeed *= distance / brakeDistance;

			// Compute force
			VEPhysics::Force force{};
			const auto desiredVelocity = MathLib::normalize(playerDirection) * desiredSpeed;
			force.linear = MathLib::normalize(desiredVelocity - dynamics.linearVelocity()) * m_limits.maxLinearForce;

			return force;
		}

	private:
		// Distance at which the option will stop
		float m_haltDistance = 0.1f;

		std::optional<EntityKnowledge> m_target;
	};

} // namespace VEAI