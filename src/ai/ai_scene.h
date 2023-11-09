#include "ai/ai_component.h"
#include "physics/motion_dynamics.h"
#include "scene/scene.h"
#include "scene/entity.h"
#include "scripting/movement/dynamic_movement_controller.h"
#include "utils/random.h"

class AIScene : public VEScene::Scene
{
public:
	using VEScene::Scene::Scene;

	void initialize() override
	{
		{ 
			// Models
			auto planeModel = loadModel("models/plane.obj");
			auto arrowModel = loadModel("models/arrow.obj");

			// Floor
			auto plane = createEntity("Plane");
			plane.addComponent<VEComponent::Mesh>(planeModel, Color::gray());
			plane.getComponent<VEComponent::Transform>().scale = { 2.0f, 2.0f, 2.0f };
			
			// Player
			auto player = createEntity("Player");
			player.addComponent<VEComponent::Mesh>(arrowModel, Color::blue());
			player.addComponent<VEPhysics::MotionDynamics>();
			player.addComponent<VEScripting::DynamicMovementController>();

			// Spawn NPCs at random locations
			int npcCount = 10; 
			for (int i = 0; i < npcCount; i++)
			{
				Vector3 randomLocation = { Random::uniformFloat(-10.0f, 10.0f), 0.0f, Random::uniformFloat(-10.0f, 10.0f) };
				auto arrow = createEntity("NPC " + std::to_string(i), randomLocation);
				arrow.addComponent<VEComponent::Mesh>(arrowModel, Color::orange());
			}
		}
		{ // Lights
			auto light1 = createEntity("Light 1", {10.0f, -10.0f, -10.0f});
			light1.addComponent<VEComponent::PointLight>(Color::white(), 100.0f);

			auto light2 = createEntity("Light 2", {-10.0f, -10.0f, -10.0f});
			light2.addComponent<VEComponent::PointLight>(Color::white(), 100.0f);

			auto light3 = createEntity("Light 2", { -10.0f, -10.0f, 10.0f });
			light3.addComponent<VEComponent::PointLight>(Color::white(), 100.0f);

			auto light4 = createEntity("Light 2", { 10.0f, -10.0f, 10.0f });
			light4.addComponent<VEComponent::PointLight>(Color::white(), 100.0f);
		}
	}
};