#include "swarm_ai.h"

#include "ai/options/steering_behaviour/steering_behaviour.h"
#include "ai/options/steering_behaviour/steering_behaviour_arrive.h"
#include "ai/options/steering_behaviour/steering_behaviour_flocking.h"
#include "ai/options/steering_behaviour/steering_behaviour_wander.h"
#include "utils/random.h"

SwarmAIComponent::SwarmAIComponent(Aegis::AI::Blackboard& blackboard) : Aegis::AI::AIComponent(blackboard)
{
	m_energy = Aegis::Random::normalFloatRange(0.0f, m_maxEnergy);
}

void SwarmAIComponent::begin()
{
	m_food = m_blackboard.get<Aegis::AI::EntityGroupKnowledge>("food");
	m_swarm = m_blackboard.get<Aegis::AI::EntityGroupKnowledge>("swarm");

	assert(m_food && "Food is not set correctly (check blackboard)");
	assert(m_swarm && "Swarm is not set correctly (check blackboard)");

	wander();
}

void SwarmAIComponent::update(float deltaSeconds)
{
	Aegis::AI::AIComponent::update(deltaSeconds);

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
	auto& food = m_food->entities[Aegis::Random::uniformInt(0, static_cast<int>(m_food->entities.size()) - 1)];
	m_optionManager.emplaceQueued<Aegis::AI::SteeringBehaviourArrive>(this, Aegis::AI::EntityKnowledge(food));
	m_optionManager.emplaceQueued<EatFood>(this);
}

void SwarmAIComponent::wander()
{
	wandering = true;

	m_optionManager.cancelActive();
	auto& blendOption = m_optionManager.emplacePrioritized<Aegis::AI::SteeringBehaviourBlend>(this);
	blendOption.add<Aegis::AI::SteeringBehaviourWander>(1.0f, this);
	blendOption.add<Aegis::AI::SteeringBehaviourFlocking>(1.0f, this, *m_swarm);
}


EatFood::EatFood(SwarmAIComponent* swarmAIComponent)
	: Aegis::AI::Option(swarmAIComponent), m_swarmAIComponent(swarmAIComponent)
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