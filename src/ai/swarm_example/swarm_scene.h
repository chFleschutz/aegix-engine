#pragma once

#include "scene/scene.h"
#include "scene/components.h"
#include "physics/motion_dynamics.h"
#include "ai/swarm_example/swarm_ai.h"
#include "scripting/movement/world_border.h"
#include "utils/random.h"

class SwarmScene : public Aegix::Scene::Scene
{
public:
	using Aegix::Scene::Scene::Scene;

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
			plane.addComponent<Aegix::Component::Mesh>(planeModel, Color::blue());
			plane.getComponent<Aegix::Component::Transform>().scale = glm::vec3{ worldSize / 10.0f };

			// Food
			int foodCount = 10;
			std::vector<Aegix::Scene::Entity> food;
			for (int i = 0; i < foodCount; i++)
			{
				float x = Random::normalFloatRange(-worldSize / 2.0f, worldSize / 2.0f);
				float z = Random::normalFloatRange(-worldSize / 2.0f, worldSize / 2.0f);
				auto foodEntity = createEntity("Food " + std::to_string(i), { x, 0.0f, z });
				foodEntity.addComponent<Aegix::Component::Mesh>(teapotModel, Color::green());
				food.emplace_back(foodEntity);
			}

			auto blackBoardEntity = createEntity("Blackboard");
			auto& blackboard = blackBoardEntity.addComponent<Aegix::AI::Blackboard>();

			// NPCs at random locations
			int npcCount = 50;
			std::vector<Aegix::Scene::Entity> npcs;
			for (int i = 0; i < npcCount; i++)
			{
				float x = Random::uniformFloat(-worldSize / 2.0f, worldSize / 2.0f);
				float z = Random::uniformFloat(-worldSize / 2.0f, worldSize / 2.0f);
				auto npc = createEntity("NPC " + std::to_string(i), { x, 0.0f, z });
				npc.addComponent<Aegix::Component::Mesh>(arrowModel, Color::red());
				npc.addComponent<Aegix::Physics::MotionDynamics>();
				npc.addComponent<SwarmAIComponent>(blackboard);
				npc.addComponent<Aegix::Scripting::WorldBorder>(glm::vec3{ worldSize / 2.0f });
				npcs.emplace_back(npc);
			}

			// Fill blackboard
			blackboard.set<Aegix::AI::EntityGroupKnowledge>("swarm", npcs);
			blackboard.set<Aegix::AI::EntityGroupKnowledge>("food", food);
		}
		{
			// Lights
			auto light1 = createEntity("Light 1", { 20.0f, -20.0f, -20.0f });
			light1.addComponent<Aegix::Component::PointLight>(Color::white(), 400.0f);

			auto light2 = createEntity("Light 2", { -20.0f, -20.0f, -20.0f });
			light2.addComponent<Aegix::Component::PointLight>(Color::white(), 400.0f);

			auto light3 = createEntity("Light 3", { -20.0f, -20.0f, 20.0f });
			light3.addComponent<Aegix::Component::PointLight>(Color::white(), 400.0f);

			auto light4 = createEntity("Light 4", { 20.0f, -20.0f, 20.0f });
			light4.addComponent<Aegix::Component::PointLight>(Color::white(), 400.0f);
		}
	}
};