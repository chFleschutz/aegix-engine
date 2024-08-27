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
		getComponent<Aegix::Component::Transform>().rotation += glm::vec3{ 0.0f, 0.0f, 1.0f } * deltaSeconds;
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
		auto helmetMesh = assetManager.createModel("damaged_helmet/DamagedHelmet.gltf");

		// MATERIALS
		auto paintingTexture = assetManager.createTexture("textures/painting.png");
		auto paintingMat = assetManager.createMaterialInstance<Aegix::Graphics::DefaultMaterial>(paintingTexture);
		paintingMat->setData({
			.shininess = 32.0f
			});

		auto metalTexture = assetManager.createTexture("textures/brushed-metal.png");
		auto metalMat = assetManager.createMaterialInstance<Aegix::Graphics::DefaultMaterial>(metalTexture);
		metalMat->setData({
			.shininess = 128.0f
			});

		auto helmetTexture = assetManager.createTexture("damaged_helmet/Default_albedo.jpg");
		auto helmetMat = assetManager.createMaterialInstance<Aegix::Graphics::DefaultMaterial>(helmetTexture);
		helmetMat->setData({
			.shininess = 32.0f
			});

		// ENTITIES
		auto plane = createEntity("Plane");
		plane.addComponent<Aegix::Component::Mesh>(planeModel);
		plane.addComponent<Aegix::Graphics::DefaultMaterial>(paintingMat);
		auto& planeTransform = plane.getComponent<Aegix::Component::Transform>();
		planeTransform.location = { 0.0f, 5.0f, 5.0f };
		
		auto floorPlane = createEntity("Floor Plane");
		floorPlane.addComponent<Aegix::Component::Mesh>(planeModel);
		floorPlane.addComponent<Aegix::Graphics::DefaultMaterial>(metalMat);
		floorPlane.getComponent<Aegix::Component::Transform>().rotation = { glm::radians(-90.0f), 0.0f, 0.0f };

		auto teapot = createEntity("Teapot");
		teapot.addComponent<Aegix::Component::Mesh>(teapotModel);
		teapot.addComponent<Aegix::Graphics::DefaultMaterial>(metalMat);
		teapot.addComponent<Rotator>();
		auto& teapotTransform = teapot.getComponent<Aegix::Component::Transform>();
		teapotTransform.scale = glm::vec3{ 2.0f, 2.0f, 2.0f };

		auto helmet = createEntity("Helmet");
		helmet.addComponent<Aegix::Component::Mesh>(helmetMesh);
		helmet.addComponent<Aegix::Graphics::DefaultMaterial>(helmetMat);
		auto& helmetTransform = helmet.getComponent<Aegix::Component::Transform>();
		helmetTransform.location = { 0.0f, 0.0f, 5.0f };
		helmetTransform.rotation = { glm::radians(180.0f), 0.0f, 0.0f };

		// LIGHTS
		assetManager.addRenderSystem<Aegix::Graphics::PointLightSystem>();

		auto light1 = createEntity("Light 1", { -7.0f, -5.0f, 5.0f });
		light1.addComponent<Aegix::Component::PointLight>(Aegix::Color::blue(), 100.0f);

		auto light2 = createEntity("Light 2", { 7.0f, -5.0f, 5.0f });
		light2.addComponent<Aegix::Component::PointLight>(Aegix::Color::green(), 100.0f);
	}
};
