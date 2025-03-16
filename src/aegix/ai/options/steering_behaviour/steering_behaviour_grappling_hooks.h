#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"

namespace Aegix::AI
{
	class SteeringBehaviourGrapplingHooks : public SteeringBehaviour
	{
	public:
		explicit SteeringBehaviourGrapplingHooks(AIComponent* aiComponent, const PathKnowledge& path)
			: SteeringBehaviour(aiComponent),
			m_path(path) {}

		virtual Aegix::Physics::Force computeForce() override
		{
			auto& transform = m_aiComponent->getComponent<Aegix::Component::Transform>();

			auto targetDistance = glm::length(m_path.path[m_currentIndex] - transform.location);
			if (targetDistance < m_distanceThreshold)
			{
				m_currentIndex++;
				if (m_currentIndex >= m_path.path.size())
				{
					stop();
					return Aegix::Physics::Force{};
				}
			}

			Aegix::Physics::Force force{};
			const auto& direction = m_path.path[m_currentIndex] - transform.location;
			force.linear = Aegix::MathLib::normalize(direction) * m_limits.maxLinearForce;
			return force;
		}

	private:
		float m_distanceThreshold = 1.0f;
		int m_currentIndex = 0;
		PathKnowledge m_path;
	};
}
