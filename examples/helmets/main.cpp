#include <aegix/engine.h>
#include <aegix/scene/components.h>
#include <aegix/scene/description.h>
#include <aegix/scripting/script_base.h>

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
		
		// ENTITIES
		auto spheres = scene.load(ASSETS_DIR "MetalRoughSpheres/MetalRoughSpheres.gltf");
		spheres.get<Transform>().location = { 0.0f, 5.0f, 5.0f };

		auto damagedHelmet = scene.load(ASSETS_DIR "DamagedHelmet/DamagedHelmet.gltf");
		damagedHelmet.get<Transform>().location = { -2.0f, 0.0f, 2.0f };

		auto scifiHelmet = scene.load(ASSETS_DIR "SciFiHelmet/ScifiHelmet.gltf");
		scifiHelmet.get<Transform>().location = { 2.0f, 0.0f, 2.0f };

		auto plane = scene.load(ASSETS_DIR "Misc/plane.obj");
		plane.get<Transform>().scale = { 2.0f, 2.0f, 2.0f };
		plane.add<ColorChanger>();
	}
};

auto main() -> int
{
	auto& engine = Aegix::Engine::instance();
	engine.loadScene<HelmetScene>();
	engine.run();
}
