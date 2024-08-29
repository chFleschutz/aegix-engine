#pragma once

#include "core/asset_manager.h"
#include "graphics/systems/pbs_render_system.h"
#include "graphics/systems/point_light_system.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "scene/scene.h"

/// @brief Scene with a teapot on a plane
/// @note Example of a custom scene
class PBSScene : public Aegix::Scene::Scene
{
public:
	/// @brief All objects in a scene are created here
	void initialize() override
	{
		auto& assetManager = Aegix::AssetManager::instance();
		assetManager.addRenderSystem<Aegix::Graphics::PointLightSystem>();

		// MODELS
		auto damagedHelmetMesh = assetManager.createModel("damaged_helmet/DamagedHelmet.gltf");
		auto scifiHelmetMesh = assetManager.createModel("scifi_helmet/ScifiHelmet.gltf");
		auto planeMesh = assetManager.createModel("models/plane.obj");

		// MATERIALS
		auto textureBlack = assetManager.createTexture(glm::vec4{ 0.0f }, 1, 1, { .format = VK_FORMAT_R8G8B8A8_UNORM });
		auto textureWhite = assetManager.createTexture(glm::vec4{ 1.0f }, 1, 1, { .format = VK_FORMAT_R8G8B8A8_UNORM });
		auto defaultNormal = assetManager.createTexture(glm::vec4{ 0.5f, 0.5f, 1.0f, 1.0f }, 1, 1, { .format = VK_FORMAT_R8G8B8A8_UNORM });
		auto planeMat = assetManager.createMaterialInstance<Aegix::Graphics::PBSMaterial>(
			textureWhite, defaultNormal, textureWhite, textureBlack, textureBlack 
		);

		auto damagedHelmetMat = assetManager.createMaterialInstance<Aegix::Graphics::PBSMaterial>(
			assetManager.createTexture("damaged_helmet/Default_albedo.jpg", { .format = VK_FORMAT_R8G8B8A8_SRGB }),
			assetManager.createTexture("damaged_helmet/Default_normal.jpg", { .format = VK_FORMAT_R8G8B8A8_UNORM }),
			assetManager.createTexture("damaged_helmet/Default_metalRoughness.jpg", { .format = VK_FORMAT_R8G8B8A8_UNORM }),
			assetManager.createTexture("damaged_helmet/Default_AO.jpg", { .format = VK_FORMAT_R8G8B8A8_UNORM }),
			assetManager.createTexture("damaged_helmet/Default_emissive.jpg", { .format = VK_FORMAT_R8G8B8A8_SRGB })
		);

		auto scifiHelmetMat = assetManager.createMaterialInstance<Aegix::Graphics::PBSMaterial>(
			assetManager.createTexture("scifi_helmet/SciFiHelmet_BaseColor.png", { .format = VK_FORMAT_R8G8B8A8_SRGB }),
			assetManager.createTexture("scifi_helmet/SciFiHelmet_Normal.png", { .format = VK_FORMAT_R8G8B8A8_UNORM }),
			assetManager.createTexture("scifi_helmet/SciFiHelmet_MetallicRoughness.png", { .format = VK_FORMAT_R8G8B8A8_UNORM }),
			assetManager.createTexture("scifi_helmet/SciFiHelmet_AmbientOcclusion.png", { .format = VK_FORMAT_R8G8B8A8_UNORM }),
			textureBlack
		);

		// ENTITIES
		auto plane = createEntity("Plane");
		plane.addComponent<Aegix::Component::Mesh>(planeMesh);
		plane.addComponent<Aegix::Graphics::PBSMaterial>(planeMat);
		plane.getComponent<Aegix::Component::Transform>().scale = { 20.0f, 20.0f, 20.0f };

		auto damagedHelmet = createEntity("Damaged Helmet", { -2.0f, 0.0f, 2.0f });
		damagedHelmet.addComponent<Aegix::Component::Mesh>(damagedHelmetMesh);
		damagedHelmet.addComponent<Aegix::Graphics::PBSMaterial>(damagedHelmetMat);
		damagedHelmet.getComponent<Aegix::Component::Transform>().rotation = { glm::radians(180.0f), 0.0f, 0.0f };

		auto scifiHelmet = createEntity("SciFi Helmet", { 2.0f, 0.0f, 2.0f });
		scifiHelmet.addComponent<Aegix::Component::Mesh>(scifiHelmetMesh);
		scifiHelmet.addComponent<Aegix::Graphics::PBSMaterial>(scifiHelmetMat);
		scifiHelmet.getComponent<Aegix::Component::Transform>().rotation = { glm::radians(90.0f), 0.0f, 0.0f };

		// LIGHTS
		auto light1 = createEntity("Light 1", { -7.0f, -5.0f, 5.0f });
		light1.addComponent<Aegix::Component::PointLight>(Aegix::Color::white(), 200.0f);

		auto light2 = createEntity("Light 2", { 7.0f, -5.0f, 5.0f });
		light2.addComponent<Aegix::Component::PointLight>(Aegix::Color::white(), 200.0f);
	}
};
