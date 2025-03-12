#include "core/engine.h"
#include "scene/description.h"

class Sponza : public Aegix::Scene::Description
{
public:
	void initialize(Aegix::Scene::Scene& scene) override
	{
		using namespace Aegix;

		// CAMERA
		scene.mainCamera().component<Transform>() = Transform{
			.location = { -9.75f, 1.2f, 5.25f},
			.rotation = glm::radians(glm::vec3{ -12.0f, 0.0f, 263.0f })
		};

		// SKYBOX
		auto& environment = scene.environment().component<Environment>();
		environment.skybox = Graphics::Texture::create(ASSETS_DIR "Environments/AutumnFieldSky.hdr");
		environment.irradiance = Graphics::Texture::createIrradiance(environment.skybox);

		// MODELS
		scene.load(ASSETS_DIR "Sponza/Sponza.gltf");

		// LIGHTS
		auto light = scene.createEntity("Point Light 1", { -5.0f, 0.3f, 7.0f });
		light.addComponent<PointLight>(glm::vec3{ 1.0f, 1.0f, 1.0f }, 50.0f);

		auto light2 = scene.createEntity("Point Light 2", { 4.0f, 0.3f, 7.0f });
		light2.addComponent<PointLight>(glm::vec3{ 1.0f, 1.0f, 1.0f }, 50.0f);

		auto light3 = scene.createEntity("Point Light 3", { -0.5f, 0.3f, 7.0f });
		light3.addComponent<PointLight>(glm::vec3{ 1.0f, 1.0f, 1.0f }, 50.0f);
	}
};

auto main() -> int
{
	auto& engine = Aegix::Engine::instance();
	engine.loadScene<Sponza>();
	engine.run();
}
