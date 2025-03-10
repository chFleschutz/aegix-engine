#pragma once

#include "graphics/systems/default_render_system.h"
#include "graphics/systems/point_light_system.h"
#include "scene/components.h"
#include "scene/description.h"
#include "utils/color.h"

class HelmetScene : public Aegix::Scene::Description
{
public:
	/// @brief All objects in a scene are created here
	void initialize(Aegix::Scene::Scene& scene) override
	{
		using namespace Aegix;

		auto& renderer = Engine::instance().renderer();
		renderer.addRenderSystem<Graphics::PointLightSystem>();

		// CAMERA
		scene.mainCamera().component<Transform>() = Transform{
			.location = { -3.0f, -6.0f, 3.0f},
			.rotation = glm::radians(glm::vec3{ -8.0f, 0.0f, 335.0f })
		};

		// ENTITIES
		auto damagedHelmet = scene.load(ASSETS_DIR "DamagedHelmet/DamagedHelmet.gltf");
		damagedHelmet.component<Transform>().location = { -2.0f, 0.0f, 2.0f };

		auto scifiHelmet = scene.load(ASSETS_DIR "SciFiHelmet/ScifiHelmet.gltf");
		scifiHelmet.component<Transform>().location = { 2.0f, 0.0f, 2.0f };

		auto plane = scene.createEntity("Plane");
		plane.component<Transform>().scale = { 20.0f, 20.0f, 20.0f };

		auto planeMesh = Graphics::StaticMesh::create(ASSETS_DIR "Misc/plane.obj");
		plane.addComponent<Mesh>(planeMesh);

		auto textureBlack = Graphics::SampledTexture::create({ 1, 1 }, glm::vec4{ 0.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		auto textureWhite = Graphics::SampledTexture::create({ 1, 1 }, glm::vec4{ 1.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		auto defaultNormal = Graphics::SampledTexture::create({ 1, 1 }, glm::vec4{ 0.5f, 0.5f, 1.0f, 0.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		auto planeMat = renderer.createMaterialInstance<Graphics::DefaultMaterial>(
			textureWhite, defaultNormal, textureWhite, textureBlack, textureBlack);
		plane.addComponent<Graphics::DefaultMaterial>(planeMat);

		// LIGHTS
		scene.ambientLight().component<AmbientLight>().intensity = 0.1f;
		scene.directionalLight().component<DirectionalLight>().intensity = 0.0f;

		auto light1 = scene.createEntity("Light 1", { 0.0f, 6.0f, 5.0f });
		light1.addComponent<PointLight>(glm::vec4{ 0.7f, 0.0f, 1.0f, 1.0f }, 200.0f);

		auto light2 = scene.createEntity("Light 2", { 7.0f, -5.0f, 5.0f });
		light2.addComponent<PointLight>(Color::blue(), 200.0f);

		auto light3 = scene.createEntity("Light 3", { -8.0f, -5.0f, 5.0f });
		light3.addComponent<PointLight>(Color::white(), 200.0f);
	}
};
