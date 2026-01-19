#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"
#include "utils/random.h"

namespace Aegis::AI
{
	class SteeringBehaviourWander : public SteeringBehaviour
	{
	public:
		explicit SteeringBehaviourWander(AIComponent* aiComponent)
			: SteeringBehaviour(aiComponent) {}

		virtual Aegis::Physics::Force computeForce() override
		{
			auto& transform = m_aiComponent->getComponent<Aegis::Component::Transform>();

			auto centerPoint = Aegis::MathLib::forward({ 0.0f, m_currentAngle, 0.0f }) * m_distance;
			m_currentAngle += Aegis::Random::uniformFloat(-m_jitter, m_jitter);
			auto boderpoint = centerPoint + (Aegis::MathLib::forward({ 0.0f, m_currentAngle, 0.0f }) * m_radius);

			Aegis::Physics::Force force;
			force.linear = Aegis::MathLib::normalize(boderpoint) * m_limits.maxLinearForce;
			return force;
		}

	private:
		float m_currentAngle = 0.0f;
		float m_radius = 5.0f;
		float m_distance = 1.0f;
		float m_jitter = glm::radians(10.0f);
	};
}
