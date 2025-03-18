#pragma once

#include "ai/blackboard.h"
#include "ai/knowledge.h"

#include <memory>

namespace Aegix::AI
{
	///@brief Base class for all considerations
	class Consideration
	{
	public:
		Consideration(Blackboard& blackboard) : m_blackboard(blackboard) {}
		virtual ~Consideration() = default;

		virtual bool evaluate() const = 0;

	protected:
		Blackboard& m_blackboard;
	};


	///@brief Consideration that checks a bool value
	class BoolConsideration : public Consideration
	{
	public:
		BoolConsideration(Blackboard& blackboard, std::string boolKey);

		virtual bool evaluate() const override;

	private:
		std::string m_key;
	};


	///@brief Consideration that checks if a float value is greater than threshold
	class ThresholdConsideration : public Consideration
	{
	public:
		///@brief Consideration that evaluates to true if the value of the given key is greater than the threshold
		ThresholdConsideration(Blackboard& blackboard, std::string floatKey, float threshold);

		virtual bool evaluate() const override;

	private:
		std::string m_key;
		float m_threshold;
	};


	///@brief Consideration that checks if the distance between two entities is less than distance
	class EntityDistanceConsideration : public Consideration
	{
	public:
		///@brief Consideration that evaluates to true if the distance between the two entities is less than the given distance
		EntityDistanceConsideration(Blackboard& blackboard, std::string entityKeyA, std::string entityKeyB, float distance);

		virtual bool evaluate() const override;

	private:
		std::string m_entityKeyA;
		std::string m_entityKeyB;
		float m_distance;
	};
}
