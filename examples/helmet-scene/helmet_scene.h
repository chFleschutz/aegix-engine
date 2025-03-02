#pragma once

#include "core/asset_manager.h"
#include "graphics/systems/default_render_system.h"
#include "graphics/systems/point_light_system.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "scene/description.h"

class HelmetScene : public Aegix::Scene::Description
{
public:
	/// @brief All objects in a scene are created here
	void initialize(Aegix::Scene::Scene& scene) override
	{
		using namespace Aegix;

		auto& assetManager = AssetManager::instance();
		assetManager.addRenderSystem<Graphics::PointLightSystem>();

		// CAMERA
		scene.mainCamera().component<Transform>() = Transform{
			.location = { -3.0f, -6.0f, 3.0f},
			.rotation = { glm::radians(-8.0f), 0.0f, glm::radians(335.0f) }
		};

		// MODELS
		auto damagedHelmetMesh = assetManager.createModel("damaged_helmet/DamagedHelmet.gltf");
		auto scifiHelmetMesh = assetManager.createModel("scifi_helmet/ScifiHelmet.gltf");
		auto planeMesh = assetManager.createModel("models/plane.obj");

		// MATERIALS
		auto textureBlack = assetManager.createTexture(glm::vec4{ 0.0f }, 1, 1, VK_FORMAT_R8G8B8A8_UNORM);
		auto textureWhite = assetManager.createTexture(glm::vec4{ 1.0f }, 1, 1, VK_FORMAT_R8G8B8A8_UNORM);
		auto defaultNormal = assetManager.createTexture(glm::vec4{ 0.5f, 0.5f, 1.0f, 1.0f }, 1, 1, VK_FORMAT_R8G8B8A8_UNORM);
		auto planeMat = assetManager.createMaterialInstance<Graphics::DefaultMaterial>(
			textureWhite, defaultNormal, textureWhite, textureBlack, textureBlack
		);

		auto damagedHelmetMat = assetManager.createMaterialInstance<Graphics::DefaultMaterial>(
			assetManager.createTexture("damaged_helmet/Default_albedo.jpg"),
			assetManager.createTexture("damaged_helmet/Default_normal.jpg", VK_FORMAT_R8G8B8A8_UNORM),
			assetManager.createTexture("damaged_helmet/Default_metalRoughness.jpg", VK_FORMAT_R8G8B8A8_UNORM),
			assetManager.createTexture("damaged_helmet/Default_AO.jpg", VK_FORMAT_R8G8B8A8_UNORM),
			assetManager.createTexture("damaged_helmet/Default_emissive.jpg")
		);

		auto scifiHelmetMat = assetManager.createMaterialInstance<Graphics::DefaultMaterial>(
			assetManager.createTexture("scifi_helmet/SciFiHelmet_BaseColor.png"),
			assetManager.createTexture("scifi_helmet/SciFiHelmet_Normal.png", VK_FORMAT_R8G8B8A8_UNORM),
			assetManager.createTexture("scifi_helmet/SciFiHelmet_MetallicRoughness.png", VK_FORMAT_R8G8B8A8_UNORM),
			assetManager.createTexture("scifi_helmet/SciFiHelmet_AmbientOcclusion.png", VK_FORMAT_R8G8B8A8_UNORM),
			textureBlack
		);

		// ENTITIES
		auto plane = scene.createEntity("Plane");
		plane.addComponent<Mesh>(planeMesh);
		plane.addComponent<Graphics::DefaultMaterial>(planeMat);
		plane.component<Transform>().scale = { 20.0f, 20.0f, 20.0f };

		auto damagedHelmet = scene.createEntity("Damaged Helmet", { -2.0f, 0.0f, 2.0f });
		damagedHelmet.addComponent<Mesh>(damagedHelmetMesh);
		damagedHelmet.addComponent<Graphics::DefaultMaterial>(damagedHelmetMat);
		damagedHelmet.component<Transform>().rotation = { glm::radians(180.0f), 0.0f, 0.0f };

		auto scifiHelmet = scene.createEntity("SciFi Helmet", { 2.0f, 0.0f, 2.0f });
		scifiHelmet.addComponent<Mesh>(scifiHelmetMesh);
		scifiHelmet.addComponent<Graphics::DefaultMaterial>(scifiHelmetMat);
		scifiHelmet.component<Transform>().rotation = { glm::radians(90.0f), 0.0f, 0.0f };

		// LIGHTS
		auto light1 = scene.createEntity("Light 1", { 0.0f, 6.0f, 5.0f });
		light1.addComponent<PointLight>(glm::vec4{ 0.7f, 0.0f, 1.0f, 1.0f }, 200.0f);

		auto light2 = scene.createEntity("Light 2", { 7.0f, -5.0f, 5.0f });
		light2.addComponent<PointLight>(Color::blue(), 200.0f);

		auto light3 = scene.createEntity("Light 3", { -8.0f, -5.0f, 5.0f });
		light3.addComponent<PointLight>(Color::white(), 200.0f);
	}
};
