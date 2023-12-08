#include "ai/test_ai_component.h"
#include "physics/motion_dynamics.h"
#include "scene/scene.h"
#include "scene/entity.h"
#include "scripting/movement/dynamic_movement_controller.h"
#include "scripting/movement/world_border.h"
#include "utils/random.h"

#include <vector>

class AIScene : public VEScene::Scene
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

			// Floor
			auto plane = createEntity("Plane");
			plane.addComponent<VEComponent::Mesh>(planeModel, Color::gray());
			plane.getComponent<VEComponent::Transform>().scale = Vector3{ worldSize / 10.0f };
			
			// Player
			auto player = createEntity("Player");
			player.addComponent<VEComponent::Mesh>(arrowModel, Color::blue());
			player.addComponent<VEPhysics::MotionDynamics>();
			player.addComponent<VEScripting::DynamicMovementController>();
			player.addComponent<VEScripting::WorldBorder>(Vector3{ worldSize / 2.0f });

			auto blackboardEntity = createEntity("Blackboard");
			auto& blackboard = blackboardEntity.addComponent<VEAI::Blackboard>();

			// NPCs at random locations
			int npcCount = 10; 
			std::vector<VEScene::Entity> npcs;
			for (int i = 0; i < npcCount; i++)
			{
				Vector3 randomLocation = { Random::uniformFloat(-10.0f, 10.0f), 0.0f, Random::uniformFloat(-10.0f, 10.0f) };
				npcs.emplace_back(createEntity("NPC " + std::to_string(i), randomLocation));
			}

			for (auto& npc : npcs)
			{
				npc.addComponent<VEComponent::Mesh>(arrowModel, Color::red());
				npc.addComponent<VEPhysics::MotionDynamics>();
				npc.addComponent<VEAI::TestAIComponent>(blackboard);
				npc.addComponent<VEScripting::WorldBorder>(Vector3{ worldSize / 2.0f });
			}

			// Fill blackboard
			blackboard.set<VEAI::EntityKnowledge>("Player", player);
			blackboard.set<VEAI::EntityGroupKnowledge>("NPCs", npcs);
		}
		{ 
			// Lights
			auto light1 = createEntity("Light 1", {20.0f, -20.0f, -20.0f});
			light1.addComponent<VEComponent::PointLight>(Color::white(), 400.0f);

			auto light2 = createEntity("Light 2", {-20.0f, -20.0f, -20.0f});
			light2.addComponent<VEComponent::PointLight>(Color::white(), 400.0f);

			auto light3 = createEntity("Light 3", { -20.0f, -20.0f, 20.0f });
			light3.addComponent<VEComponent::PointLight>(Color::white(), 400.0f);

			auto light4 = createEntity("Light 4", { 20.0f, -20.0f, 20.0f });
			light4.addComponent<VEComponent::PointLight>(Color::white(), 400.0f);
		}
	}
};