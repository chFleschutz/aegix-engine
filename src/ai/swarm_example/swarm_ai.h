#pragma once

#include "ai/ai_component.h"
#include "ai/options/option.h"


class EatFood;

class SwarmAIComponent : public VEAI::AIComponent
{
public:
	SwarmAIComponent(std::vector<VEScene::Entity> food, std::vector<VEScene::Entity> group);

	void addEnergy(float energy) { m_energy += energy; }
	float energy() const { return m_energy; }
	float maxEnergy() const { return m_maxEnergy; }

	void update(float deltaSeconds) override;

private:
	void eatFood();
	void wander();

	float m_energy;
	float m_maxEnergy = 100.0f;
	float m_energyDrainRate = 10.0f;

	bool wandering = true;

	std::vector<VEScene::Entity> m_food;
	std::vector<VEScene::Entity> m_swarm;
};


class EatFood : public VEAI::Option
{
public:
	EatFood(SwarmAIComponent* swarmAIComponent);

protected:
	virtual void updateOption(float deltaSeconds) override;

private:
	float m_energyGainRate = 20.0f;
	SwarmAIComponent* m_swarmAIComponent;
};
