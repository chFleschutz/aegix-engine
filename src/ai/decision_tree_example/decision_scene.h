#include "scene/scene.h"

#include "ai/blackboard.h"
#include "ai/knowledge.h"
#include "decision_tree_ai_component.h"
#include "physics/motion_dynamics.h"
#include "scene/entity.h"
#include "scripting/movement/dynamic_movement_controller.h"
#include "scripting/movement/world_border.h"
#include "utils/random.h"

class DecisionTreeScene : public Aegix::Scene::Scene
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

			// Floor
			auto plane = createEntity("Plane");
			plane.addComponent<Aegix::Component::Mesh>(planeModel, Color::gray());
			plane.getComponent<Aegix::Component::Transform>().scale = glm::vec3{ worldSize / 10.0f };
			
			// Player
			auto player = createEntity("Player");
			player.addComponent<Aegix::Component::Mesh>(arrowModel, Color::blue());
			player.addComponent<Aegix::Physics::MotionDynamics>();
			player.addComponent<Aegix::Scripting::DynamicMovementController>();
			player.addComponent<Aegix::Scripting::WorldBorder>(glm::vec3{ worldSize / 2.0f });

			//Blackboard
			auto blackboardEntity = createEntity("Blackboard");
			auto& blackboard = blackboardEntity.addComponent<Aegix::AI::Blackboard>();

			// NPC
			glm::vec3 randomLocation = { Random::uniformFloat(-10.0f, 10.0f), 0.0f, Random::uniformFloat(-10.0f, 10.0f) };
			auto npc = createEntity("NPC", randomLocation);
			npc.addComponent<Aegix::Component::Mesh>(arrowModel, Color::red());
			npc.addComponent<Aegix::Physics::MotionDynamics>();
			npc.addComponent<Aegix::AI::DecisionTreeAiComponent>(blackboard);
			npc.addComponent<Aegix::Scripting::WorldBorder>(glm::vec3{ worldSize / 2.0f });

			// Fill Blackboard
			blackboard.set<Aegix::AI::EntityKnowledge>("Player", player);
			blackboard.set<Aegix::AI::EntityKnowledge>("NPC", npc);
			blackboard.set<Aegix::AI::BoolKnowledge>("AtWar", false);
			blackboard.set<Aegix::AI::FloatKnowledge>("Time", 0.0f);
		}
		{ 
			// Lights
			auto light1 = createEntity("Light 1", {20.0f, -20.0f, -20.0f});
			light1.addComponent<Aegix::Component::PointLight>(Color::white(), 400.0f);

			auto light2 = createEntity("Light 2", {-20.0f, -20.0f, -20.0f});
			light2.addComponent<Aegix::Component::PointLight>(Color::white(), 400.0f);

			auto light3 = createEntity("Light 3", { -20.0f, -20.0f, 20.0f });
			light3.addComponent<Aegix::Component::PointLight>(Color::white(), 400.0f);

			auto light4 = createEntity("Light 4", { 20.0f, -20.0f, 20.0f });
			light4.addComponent<Aegix::Component::PointLight>(Color::white(), 400.0f);
		}
	}
};