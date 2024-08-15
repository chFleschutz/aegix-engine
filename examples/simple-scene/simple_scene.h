#pragma once

#include "core/asset_manager.h"
#include "graphics/systems/default_render_system.h"
#include "graphics/systems/point_light_system.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "scene/scene.h"
#include "scripting/script_base.h"

/// @brief Rotates the entity around the vertical axis
/// @note Example of a custom Component
class Rotator : public Aegix::Scripting::ScriptBase
{
protected:
	void update(float deltaSeconds) override
	{
		getComponent<Aegix::Component::Transform>().rotation += Vector3{ 0.0f, 1.0f, 0.0f } *deltaSeconds;
	}
};

/// @brief Scene with a teapot on a plane
/// @note Example of a custom scene
class SimpleScene : public Aegix::Scene::Scene
{
public:
	/// @brief All objects in a scene are created here
	void initialize() override
	{
		auto& assetManager = Aegix::AssetManager::instance();

		// MODELS

		auto teapotModel = assetManager.createModel("models/teapot.obj");
		auto planeModel = assetManager.createModel("models/plane.obj");
		auto paintingTexture = assetManager.createTexture("textures/painting.png");

		auto whiteMat = assetManager.createMaterialInstance<Aegix::Graphics::DefaultMaterial>(paintingTexture);
		whiteMat->setData({
			.color = { 1.0f, 1.0f, 1.0f, 1.0f }
			});

		auto colorfulMat = assetManager.createMaterialInstance<Aegix::Graphics::DefaultMaterial>(paintingTexture);
		colorfulMat->setData({
			.color = { 0.1f, 0.5f, 1.0f, 1.0f }
			});

		auto teapot = createEntity("Teapot");
		teapot.getComponent<Aegix::Component::Transform>().scale = Vector3{ 2.0f, 2.0f, 2.0f };
		teapot.addComponent<Aegix::Component::Mesh>(teapotModel);
		teapot.addComponent<Aegix::Graphics::DefaultMaterial>(colorfulMat);
		teapot.addComponent<Rotator>();

		auto plane = createEntity("Plane");
		plane.addComponent<Aegix::Component::Mesh>(planeModel);
		plane.addComponent<Aegix::Graphics::DefaultMaterial>(whiteMat);
		
		// LIGHTS

		assetManager.addRenderSystem<Aegix::Graphics::PointLightSystem>();

		auto light1 = createEntity("Light 1", { -7.0f, -5.0f, 0.0f });
		light1.addComponent<Aegix::Component::PointLight>(Aegix::Color::blue(), 100.0f);

		auto light2 = createEntity("Light 2", { 7.0f, -5.0f, 5.0f });
		light2.addComponent<Aegix::Component::PointLight>(Aegix::Color::green(), 100.0f);
	}
};
