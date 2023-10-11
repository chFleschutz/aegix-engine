#pragma once

#include "scene/components.h"
#include "scene/entity.h"
#include "scene/scene.h"


/// @brief Rotates the entity around the vertical axis
/// @note Example of a custom Component
//class Rotator : public vre::Component
//{
//public:
//	using vre::Component::Component;
//
//	void update(float deltaSeconds) override
//	{
//		entity().transform.rotation += glm::vec3{0.0f, 1.0f, 0.0f} *deltaSeconds;
//	}
//};

/// @brief Scene with a teapot on a plane
/// @note Example of a custom scene
class DefaultScene : public vre::Scene
{
public:
	using vre::Scene::Scene;

	/// @brief All objects in a scene are created here
	void initialize() override
	{
		{ // Models
			auto teapotModel = loadModel("models/teapot.obj");
			auto teapot = createEntity("Teapot");
			teapot.addComponent<vre::MeshComponent>(teapotModel);
			//teapot.addComponent<Rotator>();

			auto planeModel = loadModel("models/plane.obj");
			auto plane = createEntity("Plane");
			plane.addComponent<vre::MeshComponent>(planeModel);
		}
		{ // Lights
			auto light1 = createEntity("Light 1", { -1.0f, -1.0f, -1.0f });
			light1.addComponent<vre::PointLightComponent>();

			auto light2 = createEntity("Light 2", { 0.0f, -1.0f, -1.0f });
			light2.addComponent<vre::PointLightComponent>();
		}
	}
};
