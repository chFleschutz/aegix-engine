#include "swarm_ai.h"

#include "ai/options/steering_behaviour/steering_behaviour.h"
#include "ai/options/steering_behaviour/steering_behaviour_arrive.h"
#include "ai/options/steering_behaviour/steering_behaviour_flocking.h"
#include "ai/options/steering_behaviour/steering_behaviour_wander.h"
#include "utils/random.h"

SwarmAIComponent::SwarmAIComponent(VEAI::Blackboard& blackboard) : VEAI::AIComponent(blackboard)
{
	m_food = m_blackboard.get<VEAI::EntityGroupKnowledge>("food");
	m_swarm = m_blackboard.get<VEAI::EntityGroupKnowledge>("swarm");
	assert(m_food && m_swarm && "Food and Swarm have not been set (check blackboard)");

	m_energy = Random::normalFloatRange(0.0f, m_maxEnergy);
	wander();
}

void SwarmAIComponent::update(float deltaSeconds)
{
	VEAI::AIComponent::update(deltaSeconds);

	m_energy -= m_energyDrainRate * deltaSeconds;

	if (m_energy <= 0.0f and wandering)
		eatFood();

	if (m_energy >= m_maxEnergy and !wandering)
		wander();
}

void SwarmAIComponent::eatFood()
{
	wandering = false;

	m_optionManager.cancelActive();
	auto& food = m_food->entities[Random::uniformInt(0, static_cast<int>(m_food->entities.size()) - 1)];
	m_optionManager.emplaceQueued<VEAI::SteeringBehaviourArrive>(this, VEAI::EntityKnowledge(food));
	m_optionManager.emplaceQueued<EatFood>(this);
}

void SwarmAIComponent::wander()
{
	wandering = true;

	m_optionManager.cancelActive();
	auto& blendOption = m_optionManager.emplacePrioritized<VEAI::SteeringBehaviourBlend>(this);
	blendOption.add<VEAI::SteeringBehaviourWander>(1.0f, this);
	blendOption.add<VEAI::SteeringBehaviourFlocking>(1.0f, this, VEAI::EntityGroupKnowledge{ m_swarm->entities });
}


EatFood::EatFood(SwarmAIComponent* swarmAIComponent)
	: VEAI::Option(swarmAIComponent), m_swarmAIComponent(swarmAIComponent)
{
}

void EatFood::updateOption(float deltaSeconds)
{
	if (m_swarmAIComponent->energy() < m_swarmAIComponent->maxEnergy())
	{
		m_swarmAIComponent->addEnergy(m_energyGainRate * deltaSeconds);
	}
	else
	{
		stop();
	}
}