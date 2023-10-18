#include "scene/scene.h"
#include "scene/entity.h"
#include "ai/ai-component.h"

#include <random>

class AIScene : public vre::Scene
{
public:
	using vre::Scene::Scene;

	void initialize() override
	{
		{ // Models
			auto planeModel = loadModel("models/plane.obj");
			auto plane = createEntity("Plane");
			plane.addComponent<vre::MeshComponent>(planeModel);
			plane.getComponent<vre::TransformComponent>().scale = { 2.0f, 2.0f, 2.0f };

			plane.addScript<vai::AIComponent>();

			int npcCount = 10;

			auto arrowModel = loadModel("models/arrow.obj");
			// Init random generator
			std::random_device rd;	
			std::mt19937 gen(rd()); 
			std::uniform_real_distribution<> dis(-9.0, 9.0);
			for (int i = 0; i < npcCount; i++)
			{
				glm::vec3 randomLocation = { dis(gen), 0.0f, dis(gen) };
				auto arrow = createEntity("NPC " + std::to_string(i), randomLocation);
				arrow.addComponent<vre::MeshComponent>(arrowModel);
			}
		}
		{ // Lights
			auto light1 = createEntity("Light 1", {10.0f, -10.0f, -10.0f});
			light1.addComponent<vre::PointLightComponent>();
			light1.getComponent<vre::PointLightComponent>().intensity = 100.0f;

			auto light2 = createEntity("Light 2", {-10.0f, -10.0f, -10.0f});
			light2.addComponent<vre::PointLightComponent>();
			light2.getComponent<vre::PointLightComponent>().intensity = 100.0f;
		}
	}
};