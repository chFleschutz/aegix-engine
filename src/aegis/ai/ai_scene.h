#include "ai/test_ai_component.h"
#include "physics/motion_dynamics.h"
#include "scene/scene.h"
#include "scene/entity.h"
#include "scripting/movement/dynamic_movement_controller.h"
#include "scripting/movement/world_border.h"
#include "utils/random.h"

#include <vector>

class AIScene : public Aegis::Scene::Scene
{
public:
	using Aegis::Scene::Scene::Scene;

	void initialize() override
	{
		{
			const float worldSize = 50.0f;

			// Load Models
			auto planeModel = loadModel("models/plane.obj");
			auto arrowModel = loadModel("models/arrow.obj");

			// Floor
			auto plane = createEntity("Plane");
			plane.addComponent<Aegis::Component::Mesh>(planeModel, Color::gray());
			plane.getComponent<Aegis::Component::Transform>().scale = glm::vec3{ worldSize / 10.0f };
			
			// Player
			auto player = createEntity("Player");
			player.addComponent<Aegis::Component::Mesh>(arrowModel, Color::blue());
			player.addComponent<Aegis::Physics::MotionDynamics>();
			player.addComponent<Aegis::Scripting::DynamicMovementController>();
			player.addComponent<Aegis::Scripting::WorldBorder>(glm::vec3{ worldSize / 2.0f });

			auto blackboardEntity = createEntity("Blackboard");
			auto& blackboard = blackboardEntity.addComponent<Aegis::AI::Blackboard>();

			// NPCs at random locations
			int npcCount = 1; 
			std::vector<Aegis::Scene::Entity> npcs;
			for (int i = 0; i < npcCount; i++)
			{
				glm::vec3 randomLocation = { Random::uniformFloat(-10.0f, 10.0f), 0.0f, Random::uniformFloat(-10.0f, 10.0f) };
				auto npc = createEntity("NPC " + std::to_string(i), randomLocation);
				npc.addComponent<Aegis::Component::Mesh>(arrowModel, Color::red());
				npc.addComponent<Aegis::Physics::MotionDynamics>();
				npc.addComponent<Aegis::AI::TestAIComponent>(blackboard);
				npc.addComponent<Aegis::Scripting::WorldBorder>(glm::vec3{ worldSize / 2.0f });
				npcs.emplace_back(npc);
			}

			// Fill blackboard
			blackboard.set<Aegis::AI::EntityKnowledge>("Player", player);
			blackboard.set<Aegis::AI::EntityGroupKnowledge>("NPCs", npcs);
		}
		{ 
			// Lights
			auto light1 = createEntity("Light 1", {20.0f, -20.0f, -20.0f});
			light1.addComponent<Aegis::Component::PointLight>(Color::white(), 400.0f);

			auto light2 = createEntity("Light 2", {-20.0f, -20.0f, -20.0f});
			light2.addComponent<Aegis::Component::PointLight>(Color::white(), 400.0f);

			auto light3 = createEntity("Light 3", { -20.0f, -20.0f, 20.0f });
			light3.addComponent<Aegis::Component::PointLight>(Color::white(), 400.0f);

			auto light4 = createEntity("Light 4", { 20.0f, -20.0f, 20.0f });
			light4.addComponent<Aegis::Component::PointLight>(Color::white(), 400.0f);
		}
	}
};