#pragma once

#include "ai/ai_component.h"
#include "ai/options/option.h"
#include "ai/knowledge.h"

class EatFood;

class SwarmAIComponent : public Aegix::AI::AIComponent
{
public:
	SwarmAIComponent(Aegix::AI::Blackboard& blackboard);

	void addEnergy(float energy) { m_energy += energy; }
	float energy() const { return m_energy; }
	float maxEnergy() const { return m_maxEnergy; }

	void begin() override;

	void update(float deltaSeconds) override;

private:
	void eatFood();
	void wander();

	float m_energy;
	float m_maxEnergy = 100.0f;
	float m_energyDrainRate = 10.0f;

	bool wandering = true;

	Aegix::AI::EntityGroupKnowledge* m_food = nullptr;
	Aegix::AI::EntityGroupKnowledge* m_swarm = nullptr;
};


class EatFood : public Aegix::AI::Option
{
public:
	EatFood(SwarmAIComponent* swarmAIComponent);

protected:
	virtual void updateOption(float deltaSeconds) override;

private:
	float m_energyGainRate = 20.0f;
	SwarmAIComponent* m_swarmAIComponent;
};
