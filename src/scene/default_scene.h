#pragma once

#include "scene/components.h"
#include "scene/entity.h"
#include "scene/scene.h"
#include "scripting/script_base.h"
#include "core/asset_manager.h"

#include "graphics/systems/default_render_system.h"

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
class DefaultScene : public Aegix::Scene::Scene
{
public:
	/// @brief All objects in a scene are created here
	void initialize() override
	{
		{
			auto& assetManager = Aegix::AssetManager::instance();
			auto teapotModel = assetManager.createModel("models/teapot.obj");
			auto planeModel = assetManager.createModel("models/plane.obj");

			auto whiteMat = assetManager.createMaterialInstance<Aegix::Graphics::DefaultMaterial>();
			whiteMat->setData({ 
				.color = { 1.0f, 1.0f, 1.0f, 1.0f } 
				});

			auto redMat = assetManager.createMaterialInstance<Aegix::Graphics::DefaultMaterial>();
			redMat->setData({ 
				.color = { 0.0f, 1.0f, 1.0f, 1.0f } 
				});

			auto teapot = createEntity("Teapot");
			teapot.addComponent<Aegix::Component::Mesh>(teapotModel, Aegix::Color::red());
			teapot.addComponent<Aegix::Graphics::DefaultMaterial>(redMat);
			teapot.addComponent<Rotator>();

			auto plane = createEntity("Plane");
			plane.addComponent<Aegix::Component::Mesh>(planeModel, Aegix::Color::white());
			plane.addComponent<Aegix::Graphics::DefaultMaterial>(whiteMat);
		}
		{
			auto light1 = createEntity("Light 1", { -5.0f, -5.0f, 0.0f });
			light1.addComponent<Aegix::Component::PointLight>(Aegix::Color::white(), 10.0f);

			auto light2 = createEntity("Light 2", { 0.0f, -5.0f, 5.0f });
			light2.addComponent<Aegix::Component::PointLight>(Aegix::Color::white(), 10.0f);
		}
	}
};
