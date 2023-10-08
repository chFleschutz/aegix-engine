#pragma once

#include "scene/scene.h"

/// @brief Rotates the entity around the vertical axis
/// @note Example of a custom Component
class Rotator : public vre::Component
{
public:
	using vre::Component::Component;

	void update(float deltaSeconds) override
	{
		entity().transform.rotation += glm::vec3{0.0f, 1.0f, 0.0f} *deltaSeconds;
	}
};

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
			auto& teapot = createEntity(teapotModel, { 0.0f, 0.5f, 0.0f });
			teapot.addComponent<Rotator>();

			auto plane = loadModel("models/plane.obj");
			createEntity(plane, { 0.0f, 0.5f, 0.0f });
		}
		{ // Lights
			createPointLight({ -1.0f, -1.0f, -1.0f });
			createPointLight({ 0.0f, -1.0f, -1.0f });
		}
	}
};
