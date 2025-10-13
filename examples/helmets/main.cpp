#include <aegix/engine.h>
#include <aegix/scene/description.h>
#include <aegix/graphics/systems/default_render_system.h>
#include <aegix/graphics/systems/point_light_system.h>

#include <aegix/graphics/resources/material_template.h>
#include <aegix/graphics/resources/material_instance.h>
#include <scene/components.h>

#include <aegix/scripting/script_base.h>

#include "core/logging.h"

class ColorChanger : public Aegix::Scripting::ScriptBase
{
public:
	void update(float deltaSeconds) override
	{
		// Cycle through colors
		constexpr float speed = 0.5f;
		float time = static_cast<float>(glfwGetTime()) * speed;
		glm::vec3 color{
			(sin(time + 0.0f) + 1.0f) / 2.0f,
			(sin(time + 2.0f) + 1.0f) / 2.0f,
			(sin(time + 4.0f) + 1.0f) / 2.0f
		};

		auto& material = get<Aegix::Material>().instance;
		material->setParameter("albedo", color);
	}
};


/// @brief Scene with two helmets
class HelmetScene : public Aegix::Scene::Description
{
public:
	void initialize(Aegix::Scene::Scene& scene) override
	{
		using namespace Aegix;

		// SKYBOX
		auto& env = scene.environment().get<Environment>();
		env.skybox = Graphics::Texture::create(ASSETS_DIR "Environments/KloppenheimSky.hdr");
		env.irradiance = Graphics::Texture::createIrradiance(env.skybox);
		env.prefiltered = Graphics::Texture::createPrefiltered(env.skybox);

		// LIGHTS
		scene.ambientLight().get<AmbientLight>().intensity = 1.0f;
		scene.directionalLight().get<DirectionalLight>().intensity = 1.0f;
		scene.directionalLight().get<Transform>().rotation = glm::radians(glm::vec3{ 60.0f, 0.0f, 135.0f });

		// CAMERA
		scene.mainCamera().get<Transform>() = Transform{
			.location = { -3.0f, -6.0f, 3.0f},
			.rotation = glm::radians(glm::vec3{ -8.0f, 0.0f, 335.0f })
		};
		
		// ASSETS
		auto pbrMatTemplate = Engine::assets().get<Graphics::MaterialTemplate>("default/PBR_template");

		auto myMatInstance = std::make_shared<Graphics::MaterialInstance>(pbrMatTemplate);
		myMatInstance->setParameter("albedo", glm::vec3{ 1.0f, 0.0f, 0.0f });
		myMatInstance->setParameter("roughness", 0.1f);
		myMatInstance->setParameter("metallic", 1.0f);

		auto otherMatInstance = std::make_shared<Graphics::MaterialInstance>(pbrMatTemplate);
		otherMatInstance->setParameter("albedo", glm::vec3{ 0.0f, 1.0f, 0.0f });

		auto myMesh = Graphics::StaticMesh::create(ASSETS_DIR "Misc/cube.obj");
		
		// ENTITIES
		auto entity = scene.createEntity("MyEntity", { -2.0f, 0.0f, 1.0f });
		entity.add<Mesh>(myMesh);
		entity.add<Material>(myMatInstance);

		auto entity2 = scene.createEntity("MyEntity2", { 2.0f, 0.0f, 1.0f });
		entity2.add<Mesh>(myMesh);
		entity2.add<Material>(otherMatInstance);
		entity2.add<ColorChanger>();


		//auto spheres = scene.load(ASSETS_DIR "MetalRoughSpheres/MetalRoughSpheres.gltf");
		//spheres.get<Transform>().location = { 0.0f, 5.0f, 5.0f };

		//auto damagedHelmet = scene.load(ASSETS_DIR "DamagedHelmet/DamagedHelmet.gltf");
		//damagedHelmet.get<Transform>().location = { -2.0f, 0.0f, 2.0f };

		//auto scifiHelmet = scene.load(ASSETS_DIR "SciFiHelmet/ScifiHelmet.gltf");
		//scifiHelmet.get<Transform>().location = { 2.0f, 0.0f, 2.0f };

		//auto plane = scene.createEntity("Plane");
		//plane.get<Transform>().scale = { 2.0f, 2.0f, 2.0f };

		//auto planeMesh = Graphics::StaticMesh::create(ASSETS_DIR "Misc/plane.obj");
		//plane.add<Mesh>(planeMesh);

		//auto textureBlack = Graphics::Texture::create(glm::vec4{ 0.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		//auto textureWhite = Graphics::Texture::create(glm::vec4{ 1.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		//auto defaultNormal = Graphics::Texture::create(glm::vec4{ 0.5f, 0.5f, 1.0f, 0.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		//auto defaultMetalRoughness = Graphics::Texture::create(glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f }, VK_FORMAT_R8G8B8A8_UNORM);

		// TODO: Use new material system
		//auto planeMat = renderer.createMaterialInstance<Graphics::DefaultMaterial>(
		//	textureWhite, defaultNormal, defaultMetalRoughness, textureWhite, textureBlack);
		//plane.add<Graphics::DefaultMaterial>(planeMat);
	}
};

auto main() -> int
{
	auto& engine = Aegix::Engine::instance();
	engine.loadScene<HelmetScene>();
	engine.run();
}
