#include "swarm_ai.h"

#include "ai/options/steering_behaviour/steering_behaviour.h"
#include "ai/options/steering_behaviour/steering_behaviour_arrive.h"
#include "ai/options/steering_behaviour/steering_behaviour_flocking.h"
#include "ai/options/steering_behaviour/steering_behaviour_wander.h"
#include "utils/random.h"

SwarmAIComponent::SwarmAIComponent(std::vector<VEScene::Entity> food, std::vector<VEScene::Entity> group)
	: m_food(food), m_swarm(group)
{
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
	auto food = m_food[Random::uniformInt(0, m_food.size() - 1)];
	m_optionManager.emplaceQueued<VEAI::SteeringBehaviourArrive>(this, VEAI::EntityKnowledge(food));
	m_optionManager.emplaceQueued<EatFood>(this);
}

void SwarmAIComponent::wander()
{
	wandering = true;

	m_optionManager.cancelActive();
	auto& blendOption = m_optionManager.emplacePrioritized<VEAI::SteeringBehaviourBlend>(this);
	blendOption.add<VEAI::SteeringBehaviourWander>(1.0f, this);
	blendOption.add<VEAI::SteeringBehaviourFlocking>(1.0f, this, VEAI::EntityGroupKnowledge{ m_swarm });
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