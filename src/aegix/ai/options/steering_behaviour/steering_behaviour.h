#pragma once

#include "ai/ai_component.h"
#include "ai/options/option.h"
#include "physics/motion_dynamics.h"
#include "scene/components.h"
#include "utils/math_utils.h"

#include <iostream>

namespace Aegix::AI
{
	class SteeringBehaviour : public Option
	{
	public:
		struct Limits
		{
			float maxLinearForce = 10.0f;
			float maxAngularForce = 10.0f;

			float maxLinearSpeed = 10.0f;
			float maxAngularSpeed = 10.0f;
		};

		SteeringBehaviour(AIComponent* aiComponent) 
			: Option(aiComponent) {}

		void setLimits(const Limits& limits) { m_limits = limits; }

		virtual Aegix::Physics::Force computeForce() = 0;

	protected:
		virtual void updateOption(float deltaSeconds) override
		{
			assert(m_aiComponent->hasComponent<Aegix::Physics::MotionDynamics>() && "SteeringBehaviour needs MotionDynamics component");
			auto& dynamics = m_aiComponent->getComponent<Aegix::Physics::MotionDynamics>();
			
			dynamics.addForce(computeForce());
		}

		Limits m_limits;
	};


	class SteeringBehaviourBlend : public Option
	{
	public:
		explicit SteeringBehaviourBlend(AIComponent* aiComponent) 
			: Option(aiComponent) {}

		void add(std::unique_ptr<SteeringBehaviour> behaviour, float weight = 1.0f)
		{
			m_weightedBehaviours.push_back({ std::move(behaviour), weight });
		} 

		template<typename T, typename... Args>
		void add(float weight, Args&&... args)
		{
			m_weightedBehaviours.push_back({ std::make_unique<T>(std::forward<Args>(args)...), weight });
		}

	protected:
		virtual void updateOption(float deltaSeconds) override
		{
			assert(m_aiComponent->hasComponent<Aegix::Physics::MotionDynamics>() && "SteeringBehaviour needs MotionDynamics component");
			auto& dynamics = m_aiComponent->getComponent<Aegix::Physics::MotionDynamics>();

			Aegix::Physics::Force force{};
			for (auto& weightedBehaviour : m_weightedBehaviours)
			{
				force += weightedBehaviour.behaviour->computeForce() * weightedBehaviour.weight;
			}

			dynamics.addForce(force);
		}

	private:
		struct WeightedBehaviour
		{
			std::unique_ptr<SteeringBehaviour> behaviour;
			float weight;
		};

		std::vector<WeightedBehaviour> m_weightedBehaviours;
	};
}
