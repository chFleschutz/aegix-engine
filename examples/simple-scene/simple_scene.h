#pragma once

#include "core/asset_manager.h"
#include "graphics/systems/default_render_system.h"
#include "graphics/systems/point_light_system.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "scene/scene.h"
#include "scripting/script_base.h"
#include "utils/random.h"

/// @brief Rotates the entity around the vertical axis
/// @note Example of a custom Component
class Rotator : public Aegix::Scripting::ScriptBase
{
protected:
	void update(float deltaSeconds) override
	{
		component<Aegix::Transform>().rotation += glm::vec3{ 0.0f, 0.0f, 1.0f } * deltaSeconds;
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
		auto teapotMesh = assetManager.createModel("models/teapot.obj");
		auto planeMesh = assetManager.createModel("models/plane.obj");

		// MATERIALS
		auto textureBlack = assetManager.createTexture(glm::vec4{ 0.0f }, 1, 1, VK_FORMAT_R8G8B8A8_UNORM);
		auto textureWhite = assetManager.createTexture(glm::vec4{ 1.0f }, 1, 1, VK_FORMAT_R8G8B8A8_UNORM);
		auto defaultNormal = assetManager.createTexture(glm::vec4{ 0.5f, 0.5f, 1.0f, 1.0f }, 1, 1, VK_FORMAT_R8G8B8A8_UNORM);

		auto paintingTexture = assetManager.createTexture("textures/painting.png");
		auto paintingMat = assetManager.createMaterialInstance<Aegix::Graphics::DefaultMaterial>(
			paintingTexture, defaultNormal, textureWhite, textureBlack, textureBlack);

		auto metalTexture = assetManager.createTexture("textures/brushed-metal.png");
		auto metalMat = assetManager.createMaterialInstance<Aegix::Graphics::DefaultMaterial>(
			metalTexture, defaultNormal, textureWhite, textureBlack, textureBlack);

		// ENTITIES
		auto floorPlane = createEntity("Floor Plane");
		floorPlane.addComponent<Aegix::Mesh>(planeMesh);
		floorPlane.addComponent<Aegix::Graphics::DefaultMaterial>(paintingMat);
		floorPlane.component<Aegix::Transform>().scale = glm::vec3{ 2.0f, 2.0f, 2.0f };

		auto teapot = createEntity("Teapot");
		teapot.addComponent<Aegix::Mesh>(teapotMesh);
		teapot.addComponent<Aegix::Graphics::DefaultMaterial>(metalMat);
		teapot.addComponent<Rotator>();
		teapot.component<Aegix::Transform>().scale = glm::vec3{ 2.0f, 2.0f, 2.0f };

		// LIGHTS
		assetManager.addRenderSystem<Aegix::Graphics::PointLightSystem>();

		constexpr int lightCount = 32;
		constexpr float lightRadius = 10.0f;
		// Ring of lights
		for (int i = 0; i < lightCount; i++)
		{
			float x = lightRadius * cos(glm::radians(360.0f / lightCount * i));
			float y = lightRadius * sin(glm::radians(360.0f / lightCount * i));
			auto light = createEntity("Light " + std::to_string(i), { x, y, 3.0f });

			float r = Aegix::Random::uniformFloat(0.0f, 1.0f);
			float g = Aegix::Random::uniformFloat(0.0f, 1.0f);
			float b = Aegix::Random::uniformFloat(0.0f, 1.0f);
			light.addComponent<Aegix::PointLight>(glm::vec3{ r, g, b }, 200.0f);
		}
	}
};
