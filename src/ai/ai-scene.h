#include "scene/scene.h"
#include "scene/entity.h"
#include "ai/ai-component.h"
#include "utils/random.h"

class AIScene : public VEScene::Scene
{
public:
	using VEScene::Scene::Scene;

	void initialize() override
	{
		{ // Models
			auto planeModel = loadModel("models/plane.obj");
			auto plane = createEntity("Plane");
			plane.addComponent<VEComponents::MeshComponent>(planeModel);
			plane.getComponent<VEComponents::TransformComponent>().scale = { 2.0f, 2.0f, 2.0f };
			
			plane.addScript<vai::AIComponent>(); // TODO: Remove (for testing)

			// Spawn NPCs at random locations
			auto arrowModel = loadModel("models/arrow.obj");
			int npcCount = 10; 
			for (int i = 0; i < npcCount; i++)
			{
				Vector3 randomLocation = { Random::uniformFloat(-10.0f, 10.0f), 0.0f, Random::uniformFloat(-10.0f, 10.0f) };
				auto arrow = createEntity("NPC " + std::to_string(i), randomLocation);
				arrow.addComponent<VEComponents::MeshComponent>(arrowModel);
			}
		}
		{ // Lights
			auto light1 = createEntity("Light 1", {10.0f, -10.0f, -10.0f});
			light1.addComponent<VEComponents::PointLightComponent>(Color::white(), 100.0f);

			auto light2 = createEntity("Light 2", {-10.0f, -10.0f, -10.0f});
			light2.addComponent<VEComponents::PointLightComponent>(Color::white(), 100.0f);
		}
	}
};