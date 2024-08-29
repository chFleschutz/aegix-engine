#pragma once

#include "core/asset_manager.h"
#include "graphics/systems/pbs_render_system.h"
#include "graphics/systems/point_light_system.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "scene/scene.h"

class PBSScene : public Aegix::Scene::Scene
{
public:
	/// @brief All objects in a scene are created here
	void initialize() override
	{
		using namespace Aegix;

		auto& assetManager = AssetManager::instance();
		//assetManager.addRenderSystem<Graphics::PointLightSystem>(); // Uncomment to visualize point lights

		// CAMERA
		auto& cameraTransform = camera().getComponent<Component::Transform>();
		cameraTransform.location = { -3.0f, -6.0f, 3.0f };
		cameraTransform.rotation = { glm::radians(-8.0f), 0.0f, glm::radians(335.0f) };

		// MODELS
		auto damagedHelmetMesh = assetManager.createModel("damaged_helmet/DamagedHelmet.gltf");
		auto scifiHelmetMesh = assetManager.createModel("scifi_helmet/ScifiHelmet.gltf");
		auto planeMesh = assetManager.createModel("models/plane.obj");

		// MATERIALS
		auto textureBlack = assetManager.createTexture(glm::vec4{ 0.0f }, 1, 1, { .format = VK_FORMAT_R8G8B8A8_UNORM });
		auto textureWhite = assetManager.createTexture(glm::vec4{ 1.0f }, 1, 1, { .format = VK_FORMAT_R8G8B8A8_UNORM });
		auto defaultNormal = assetManager.createTexture(glm::vec4{ 0.5f, 0.5f, 1.0f, 1.0f }, 1, 1, { .format = VK_FORMAT_R8G8B8A8_UNORM });
		auto planeMat = assetManager.createMaterialInstance<Graphics::PBSMaterial>(
			textureWhite, defaultNormal, textureWhite, textureBlack, textureBlack
		);

		auto damagedHelmetMat = assetManager.createMaterialInstance<Graphics::PBSMaterial>(
			assetManager.createTexture("damaged_helmet/Default_albedo.jpg", { .format = VK_FORMAT_R8G8B8A8_SRGB }),
			assetManager.createTexture("damaged_helmet/Default_normal.jpg", { .format = VK_FORMAT_R8G8B8A8_UNORM }),
			assetManager.createTexture("damaged_helmet/Default_metalRoughness.jpg", { .format = VK_FORMAT_R8G8B8A8_UNORM }),
			assetManager.createTexture("damaged_helmet/Default_AO.jpg", { .format = VK_FORMAT_R8G8B8A8_UNORM }),
			assetManager.createTexture("damaged_helmet/Default_emissive.jpg", { .format = VK_FORMAT_R8G8B8A8_SRGB })
		);

		auto scifiHelmetMat = assetManager.createMaterialInstance<Graphics::PBSMaterial>(
			assetManager.createTexture("scifi_helmet/SciFiHelmet_BaseColor.png", { .format = VK_FORMAT_R8G8B8A8_SRGB }),
			assetManager.createTexture("scifi_helmet/SciFiHelmet_Normal.png", { .format = VK_FORMAT_R8G8B8A8_UNORM }),
			assetManager.createTexture("scifi_helmet/SciFiHelmet_MetallicRoughness.png", { .format = VK_FORMAT_R8G8B8A8_UNORM }),
			assetManager.createTexture("scifi_helmet/SciFiHelmet_AmbientOcclusion.png", { .format = VK_FORMAT_R8G8B8A8_UNORM }),
			textureBlack
		);

		// ENTITIES
		auto plane = createEntity("Plane");
		plane.addComponent<Component::Mesh>(planeMesh);
		plane.addComponent<Graphics::PBSMaterial>(planeMat);
		plane.getComponent<Component::Transform>().scale = { 20.0f, 20.0f, 20.0f };

		auto damagedHelmet = createEntity("Damaged Helmet", { -2.0f, 0.0f, 2.0f });
		damagedHelmet.addComponent<Component::Mesh>(damagedHelmetMesh);
		damagedHelmet.addComponent<Graphics::PBSMaterial>(damagedHelmetMat);
		damagedHelmet.getComponent<Component::Transform>().rotation = { glm::radians(180.0f), 0.0f, 0.0f };

		auto scifiHelmet = createEntity("SciFi Helmet", { 2.0f, 0.0f, 2.0f });
		scifiHelmet.addComponent<Component::Mesh>(scifiHelmetMesh);
		scifiHelmet.addComponent<Graphics::PBSMaterial>(scifiHelmetMat);
		scifiHelmet.getComponent<Component::Transform>().rotation = { glm::radians(90.0f), 0.0f, 0.0f };

		// LIGHTS
		auto light1 = createEntity("Light 1", { 0.0f, 6.0f, 5.0f });
		light1.addComponent<Component::PointLight>(glm::vec4{ 0.7f, 0.0f, 1.0f, 1.0f }, 200.0f);

		auto light2 = createEntity("Light 2", { 7.0f, -5.0f, 5.0f });
		light2.addComponent<Component::PointLight>(Color::blue(), 200.0f);

		auto light3 = createEntity("Light 3", { -8.0f, -5.0f, 5.0f });
		light3.addComponent<Component::PointLight>(Color::white(), 200.0f);
	}
};
