#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"
#include "ai/options/steering_behaviour/steering_behaviour_cohesion.h"
#include "ai/options/steering_behaviour/steering_behaviour_separation.h"
#include "ai/options/steering_behaviour/steering_behaviour_velocity_matching.h"
#include "ai/knowledge.h"

#include <memory>

namespace Aegix::AI
{
	/// @brief Combines the cohesion, separation and velocity matching behaviours into the flocking steering behaviour.
	class SteeringBehaviourFlocking : public SteeringBehaviour
	{
	public:
		SteeringBehaviourFlocking(AIComponent* aiComponent, const EntityGroupKnowledge& group)
			: SteeringBehaviour(aiComponent),
			m_cohesion(aiComponent, group),
			m_separation(aiComponent, group),
			m_velocityMatching(aiComponent, group) {}

		~SteeringBehaviourFlocking() = default;

		void setWeights(float cohesion, float separation, float velocityMatching)
		{
			m_cohesionWeight = cohesion;
			m_separationWeight = separation;
			m_velocityMatchingWeight = velocityMatching;
		}

		virtual Aegix::Physics::Force computeForce() override
		{
			Aegix::Physics::Force force{};
			force += m_cohesion.computeForce() * m_cohesionWeight;
			force += m_separation.computeForce() * m_separationWeight;
			force += m_velocityMatching.computeForce() * m_velocityMatchingWeight;
			return force;
		}

	private:
		float m_cohesionWeight = 1.0f;
		float m_separationWeight = 1.0f;
		float m_velocityMatchingWeight = 1.0f;

		SteeringBehaviourCohesion m_cohesion;
		SteeringBehaviourSeparation m_separation;
		SteeringBehaviourVelocityMatching m_velocityMatching;
	};
}
