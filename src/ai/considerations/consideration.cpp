#include "consideration.h"

#include "scene/components.h"
#include "utils/math_utils.h"

#include <cassert>


namespace Aegix::AI
{
	BoolConsideration::BoolConsideration(Blackboard& blackboard, std::string key)
		: Consideration(blackboard), 
		m_key(std::move(key)) 
	{
	}

	bool BoolConsideration::evaluate() const
	{
		auto value = m_blackboard.get<BoolKnowledge>(m_key);
		assert(value != nullptr && "Key does not exist in blackboard");
		
		return value->value;
	}

	ThresholdConsideration::ThresholdConsideration(Blackboard& blackboard, std::string key, float threshold)
		: Consideration(blackboard),
		m_key(std::move(key)),
		m_threshold(threshold)
	{
	}

	bool ThresholdConsideration::evaluate() const
	{
		auto value = m_blackboard.get<FloatKnowledge>(m_key);
		assert(value != nullptr && "Key does not exist in blackboard");
		
		return value->value >= m_threshold;
	}

	EntityDistanceConsideration::EntityDistanceConsideration(Blackboard& blackboard, std::string entityKeyA, std::string entityKeyB, float distance)
		: Consideration(blackboard),
		m_entityKeyA(std::move(entityKeyA)),
		m_entityKeyB(std::move(entityKeyB)),
		m_distance(distance)
	{
	}

	bool EntityDistanceConsideration::evaluate() const
	{
		auto entityA = m_blackboard.get<EntityKnowledge>(m_entityKeyA);
		auto entityB = m_blackboard.get<EntityKnowledge>(m_entityKeyB);
		assert(entityA && entityB && "Entity key does not exist in blackboard");

		auto& positionA = entityA->entity.getComponent<Aegix::Component::Transform>().location;
		auto& positionB = entityB->entity.getComponent<Aegix::Component::Transform>().location;
		return glm::distance(positionA, positionB) < m_distance;
	}
}
