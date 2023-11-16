#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"
#include "utils/random.h"

namespace VEAI
{
	class SteeringBehaviourWander : public SteeringBehaviour
	{
	public:
		explicit SteeringBehaviourWander(AIComponent* aiComponent)
			: SteeringBehaviour(aiComponent) {}

		virtual VEPhysics::Force computeForce() override
		{
			auto& transform = m_aiComponent->getComponent<VEComponent::Transform>();

			auto centerPoint = MathLib::forward({ 0.0f, m_currentAngle, 0.0f }) * m_distance;
			m_currentAngle += Random::uniformFloat(-m_jitter, m_jitter);
			auto boderpoint = centerPoint + (MathLib::forward({ 0.0f, m_currentAngle, 0.0f }) * m_radius);

			VEPhysics::Force force;
			force.linear = MathLib::normalize(boderpoint) * m_limits.maxLinearForce;
			return force;
		}

	private:
		float m_currentAngle = 0.0f;
		float m_radius = 5.0f;
		float m_distance = 1.0f;
		float m_jitter = glm::radians(10.0f);
	};

} // namespace VEAI
