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
		auto helmetMesh = assetManager.createModel("damaged_helmet/DamagedHelmet.gltf");

		// MATERIALS
		auto helmetMat = assetManager.createMaterialInstance<Aegix::Graphics::PBSMaterial>(
			assetManager.createTexture("damaged_helmet/Default_albedo.jpg", { .format = VK_FORMAT_R8G8B8A8_SRGB }),
			assetManager.createTexture("damaged_helmet/Default_normal.jpg", { .format = VK_FORMAT_R8G8B8A8_UNORM }),
			assetManager.createTexture("damaged_helmet/Default_metalRoughness.jpg", { .format = VK_FORMAT_R8G8B8A8_UNORM }),
			assetManager.createTexture("damaged_helmet/Default_AO.jpg", { .format = VK_FORMAT_R8G8B8A8_UNORM })
		);

		// ENTITIES
		auto helmet = createEntity("Helmet");
		helmet.addComponent<Aegix::Component::Mesh>(helmetMesh);
		helmet.addComponent<Aegix::Graphics::PBSMaterial>(helmetMat);
		auto& helmetTransform = helmet.getComponent<Aegix::Component::Transform>();
		helmetTransform.rotation = { glm::radians(180.0f), 0.0f, 0.0f };

		// LIGHTS
		auto light1 = createEntity("Light 1", { -7.0f, -5.0f, 5.0f });
		light1.addComponent<Aegix::Component::PointLight>(Aegix::Color::blue(), 100.0f);

		auto light2 = createEntity("Light 2", { 7.0f, -5.0f, 5.0f });
		light2.addComponent<Aegix::Component::PointLight>(Aegix::Color::green(), 100.0f);
	}
};
