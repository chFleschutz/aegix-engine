#include "scene/scene.h"
#include "scene/entity.h"

class AIScene : public vre::Scene
{
public:
	using vre::Scene::Scene;

	void initialize() override
	{
		{ // Models
			auto teapotModel = loadModel("models/arrow.obj");
			auto teapot = createEntity("Teapot");
			teapot.addComponent<vre::MeshComponent>(teapotModel);

			auto planeModel = loadModel("models/plane.obj");
			auto plane = createEntity("Plane");
			plane.addComponent<vre::MeshComponent>(planeModel);
		}
		{ // Lights
			auto light1 = createEntity("Light 1", { -10.0f, -10.0f, -1.0f });
			light1.addComponent<vre::PointLightComponent>();
			light1.getComponent<vre::PointLightComponent>().intensity = 100.0f;

			auto light2 = createEntity("Light 2", { 0.0f, -10.0f, -10.0f });
			light2.addComponent<vre::PointLightComponent>();
			light2.getComponent<vre::PointLightComponent>().intensity = 100.0f;
		}
	}
};