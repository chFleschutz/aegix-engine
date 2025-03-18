#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"
#include "utils/random.h"

namespace Aegix::AI
{
	class SteeringBehaviourWander : public SteeringBehaviour
	{
	public:
		explicit SteeringBehaviourWander(AIComponent* aiComponent)
			: SteeringBehaviour(aiComponent) {}

		virtual Aegix::Physics::Force computeForce() override
		{
			auto& transform = m_aiComponent->getComponent<Aegix::Component::Transform>();

			auto centerPoint = Aegix::MathLib::forward({ 0.0f, m_currentAngle, 0.0f }) * m_distance;
			m_currentAngle += Aegix::Random::uniformFloat(-m_jitter, m_jitter);
			auto boderpoint = centerPoint + (Aegix::MathLib::forward({ 0.0f, m_currentAngle, 0.0f }) * m_radius);

			Aegix::Physics::Force force;
			force.linear = Aegix::MathLib::normalize(boderpoint) * m_limits.maxLinearForce;
			return force;
		}

	private:
		float m_currentAngle = 0.0f;
		float m_radius = 5.0f;
		float m_distance = 1.0f;
		float m_jitter = glm::radians(10.0f);
	};
}
