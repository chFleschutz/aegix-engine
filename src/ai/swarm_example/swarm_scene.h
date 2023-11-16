#pragma once

#include "scene/scene.h"
#include "scene/components.h"
#include "physics/motion_dynamics.h"
#include "ai/swarm_example/swarm_ai.h"
#include "scripting/movement/world_border.h"
#include "utils/random.h"

class SwarmScene : public VEScene::Scene
{
public:
	using VEScene::Scene::Scene;

	void initialize() override
	{
		{
			const float worldSize = 50.0f;

			// Load Models
			auto planeModel = loadModel("models/plane.obj");
			auto arrowModel = loadModel("models/arrow.obj");
			auto teapotModel = loadModel("models/teapot.obj");

			// Floor
			auto plane = createEntity("Plane");
			plane.addComponent<VEComponent::Mesh>(planeModel, Color::blue());
			plane.getComponent<VEComponent::Transform>().scale = Vector3{ worldSize / 10.0f };

			// Food
			int foodCount = 10;
			std::vector<VEScene::Entity> food;
			for (int i = 0; i < foodCount; i++)
			{
				float x = Random::normalFloatRange(-worldSize / 2.0f, worldSize / 2.0f);
				float z = Random::normalFloatRange(-worldSize / 2.0f, worldSize / 2.0f);
				auto foodEntity = createEntity("Food " + std::to_string(i), { x, 0.0f, z });
				foodEntity.addComponent<VEComponent::Mesh>(teapotModel, Color::green());
				food.emplace_back(foodEntity);
			}

			// NPCs at random locations
			int npcCount = 50;
			std::vector<VEScene::Entity> npcs;
			for (int i = 0; i < npcCount; i++)
			{
				float x = Random::uniformFloat(-worldSize / 2.0f, worldSize / 2.0f);
				float z = Random::uniformFloat(-worldSize / 2.0f, worldSize / 2.0f);
				npcs.emplace_back(createEntity("NPC " + std::to_string(i), { x, 0.0f, z }));
			}

			for (auto& npc : npcs)
			{
				npc.addComponent<VEComponent::Mesh>(arrowModel, Color::red());
				npc.addComponent<VEPhysics::MotionDynamics>();
				npc.addComponent<SwarmAIComponent>(food, npcs);
				npc.addComponent<VEScripting::WorldBorder>(Vector3{ worldSize / 2.0f });
			}
		}
		{
			// Lights
			auto light1 = createEntity("Light 1", { 20.0f, -20.0f, -20.0f });
			light1.addComponent<VEComponent::PointLight>(Color::white(), 400.0f);

			auto light2 = createEntity("Light 2", { -20.0f, -20.0f, -20.0f });
			light2.addComponent<VEComponent::PointLight>(Color::white(), 400.0f);

			auto light3 = createEntity("Light 3", { -20.0f, -20.0f, 20.0f });
			light3.addComponent<VEComponent::PointLight>(Color::white(), 400.0f);

			auto light4 = createEntity("Light 4", { 20.0f, -20.0f, 20.0f });
			light4.addComponent<VEComponent::PointLight>(Color::white(), 400.0f);
		}
	}
};