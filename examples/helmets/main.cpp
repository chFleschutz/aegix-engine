#include <aegix/engine.h>
#include <aegix/scene/description.h>
#include <aegix/graphics/systems/default_render_system.h>
#include <aegix/graphics/systems/point_light_system.h>

/// @brief Scene with two helmets
class HelmetScene : public Aegix::Scene::Description
{
public:
	void initialize(Aegix::Scene::Scene& scene) override
	{
		using namespace Aegix;

		auto& renderer = Engine::instance().renderer();
		renderer.addRenderSystem<Graphics::PointLightSystem>();

		// SKYBOX
		auto& env = scene.environment().get<Environment>();
		env.skybox = Graphics::Texture::create(ASSETS_DIR "Environments/KloppenheimSky.hdr");
		env.irradiance = Graphics::Texture::createIrradiance(env.skybox);
		env.prefiltered = Graphics::Texture::createPrefiltered(env.skybox);

		// CAMERA
		scene.mainCamera().get<Transform>() = Transform{
			.location = { -3.0f, -6.0f, 3.0f},
			.rotation = glm::radians(glm::vec3{ -8.0f, 0.0f, 335.0f })
		};

		// ENTITIES
		auto spheres = scene.load(ASSETS_DIR "MetalRoughSpheres/MetalRoughSpheres.gltf");
		spheres.get<Transform>().location = { 0.0f, 5.0f, 5.0f };

		auto damagedHelmet = scene.load(ASSETS_DIR "DamagedHelmet/DamagedHelmet.gltf");
		damagedHelmet.get<Transform>().location = { -2.0f, 0.0f, 2.0f };

		auto scifiHelmet = scene.load(ASSETS_DIR "SciFiHelmet/ScifiHelmet.gltf");
		scifiHelmet.get<Transform>().location = { 2.0f, 0.0f, 2.0f };

		auto plane = scene.createEntity("Plane");
		plane.get<Transform>().scale = { 2.0f, 2.0f, 2.0f };

		auto planeMesh = Graphics::StaticMesh::create(ASSETS_DIR "Misc/plane.obj");
		plane.add<Mesh>(planeMesh);

		auto textureBlack = Graphics::Texture::create(glm::vec4{ 0.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		auto textureWhite = Graphics::Texture::create(glm::vec4{ 1.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		auto defaultNormal = Graphics::Texture::create(glm::vec4{ 0.5f, 0.5f, 1.0f, 0.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		auto defaultMetalRoughness = Graphics::Texture::create(glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		auto planeMat = renderer.createMaterialInstance<Graphics::DefaultMaterial>(
			textureWhite, defaultNormal, defaultMetalRoughness, textureWhite, textureBlack);
		plane.add<Graphics::DefaultMaterial>(planeMat);

		// LIGHTS
		scene.ambientLight().get<AmbientLight>().intensity = 1.0f;
		scene.directionalLight().get<DirectionalLight>().intensity = 1.0f;
		scene.directionalLight().get<Transform>().rotation = glm::radians(glm::vec3{ 60.0f, 0.0f, 135.0f });
	}
};

auto main() -> int
{
	auto& engine = Aegix::Engine::instance();
	engine.loadScene<HelmetScene>();
	engine.run();
}
