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
		auto& env = scene.environment().component<Environment>();
		env.skybox = Graphics::Texture::create(ASSETS_DIR "Environments/KloppenheimSky.hdr");
		env.irradiance = Graphics::Texture::createIrradiance(env.skybox);
		env.prefiltered = Graphics::Texture::createPrefiltered(env.skybox);

		// MODELS
		scene.load(ASSETS_DIR "Sponza/Sponza.gltf");

		// LIGHTS
		scene.ambientLight().component<AmbientLight>().intensity = 0.5f;
		scene.directionalLight().component<DirectionalLight>().intensity = 1.0f;
	}
};

auto main() -> int
{
	auto& engine = Aegix::Engine::instance();
	engine.loadScene<Sponza>();
	engine.run();
}
