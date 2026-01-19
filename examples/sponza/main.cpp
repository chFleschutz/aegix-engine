#include <aegis/engine.h>
#include <aegis/scene/description.h>
#include <aegis/scene/components.h>

class Sponza : public Aegis::Scene::Description
{
public:
	void initialize(Aegis::Scene::Scene& scene) override
	{
		using namespace Aegis;

		// CAMERA
		scene.mainCamera().get<Transform>() = Transform{
			.location = { -9.75f, 1.2f, 5.25f},
			.rotation = glm::radians(glm::vec3{ -12.0f, 0.0f, 263.0f })
		};

		// SKYBOX
		auto& env = scene.environment().get<Environment>();
		env.skybox = Graphics::Texture::create(ASSETS_DIR "Environments/KloppenheimSky.hdr");
		env.irradiance = Graphics::Texture::createIrradiance(env.skybox);
		env.prefiltered = Graphics::Texture::createPrefiltered(env.skybox);

		// MODELS
		scene.load(ASSETS_DIR "Sponza/Sponza.gltf");

		// LIGHTS
		scene.ambientLight().get<AmbientLight>().intensity = 0.5f;
		scene.directionalLight().get<DirectionalLight>().intensity = 1.0f;
	}
};

auto main() -> int
{
	auto& engine = Aegis::Engine::instance();
	engine.loadScene<Sponza>();
	engine.run();
}
