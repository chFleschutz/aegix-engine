#pragma once

#include "scene/components.h"
#include "scene/script_component_base.h"
#include "scene/entity.h"
#include "scene/scene.h"

/// @brief Rotates the entity around the vertical axis
/// @note Example of a custom Component
class Rotator : public VEScripting::ScriptBase
{
protected:
	void update(float deltaSeconds) override
	{
		getComponent<VEComponent::Transform>().rotation += Vector3{ 0.0f, 1.0f, 0.0f } * deltaSeconds;
	}
};

/// @brief Scene with a teapot on a plane
/// @note Example of a custom scene
class DefaultScene : public VEScene::Scene
{
public:
	using VEScene::Scene::Scene;

	/// @brief All objects in a scene are created here
	void initialize() override
	{
		{ // Models 
			auto teapotModel = loadModel("models/teapot.obj");
			auto teapot = createEntity("Teapot");
			teapot.addComponent<VEComponent::Mesh>(teapotModel, Color::red());
			teapot.addScript<Rotator>();
			
			auto planeModel = loadModel("models/plane.obj");
			auto plane = createEntity("Plane");
			plane.addComponent<VEComponent::Mesh>(planeModel, Color::white());
		}
		{ // Lights
			auto light1 = createEntity("Light 1", { -1.0f, -1.0f, -1.0f });
			light1.addComponent<VEComponent::PointLight>();

			auto light2 = createEntity("Light 2", { 0.0f, -1.0f, -1.0f });
			light2.addComponent<VEComponent::PointLight>();
		}
	}
};
