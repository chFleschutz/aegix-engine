#include <engine.h>
#include <graphics/systems/default_render_system.h>
#include <graphics/systems/point_light_system.h>
#include <math/random.h>
#include <scene/description.h>
#include <scripting/script_base.h>

/// @brief Example script to rotate the entity around the up axis
class Rotator : public Aegix::Scripting::ScriptBase
{
protected:
	void update(float deltaSeconds) override
	{
		auto& transform = component<Aegix::Transform>();
		transform.rotation *= glm::angleAxis(deltaSeconds, Aegix::Math::World::UP);
	}
};

/// @brief Simple scene with a teapot, a plane and a bunch of lights
class SimpleScene : public Aegix::Scene::Description
{
public:
	void initialize(Aegix::Scene::Scene& scene) override
	{
		using namespace Aegix;

		auto& renderer = Engine::instance().renderer();
		renderer.addRenderSystem<Graphics::PointLightSystem>();

		// MODELS
		auto teapotMesh = Graphics::StaticMesh::create(ASSETS_DIR "Misc/teapot.obj");
		auto planeMesh = Graphics::StaticMesh::create(ASSETS_DIR "Misc/plane.obj");

		// MATERIALS
		auto textureBlack = Graphics::Texture::create(glm::vec4{ 0.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		auto textureWhite = Graphics::Texture::create(glm::vec4{ 1.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		auto defaultNormal = Graphics::Texture::create(glm::vec4{ 0.5f, 0.5f, 1.0f, 1.0f }, VK_FORMAT_R8G8B8A8_UNORM);

		auto paintingTexture = Graphics::Texture::create(ASSETS_DIR "Misc/painting.png", VK_FORMAT_R8G8B8A8_SRGB);
		auto paintingMat = renderer.createMaterialInstance<Graphics::DefaultMaterial>(
			paintingTexture, defaultNormal, textureWhite, textureBlack, textureBlack);

		auto metalTexture = Graphics::Texture::create(ASSETS_DIR "Misc/brushed-metal.png", VK_FORMAT_R8G8B8A8_SRGB);
		auto metalMat = renderer.createMaterialInstance<Graphics::DefaultMaterial>(
			metalTexture, defaultNormal, textureWhite, textureBlack, textureBlack);

		// ENTITIES
		auto floorPlane = scene.createEntity("Floor Plane");
		floorPlane.addComponent<Mesh>(planeMesh);
		floorPlane.addComponent<Graphics::DefaultMaterial>(paintingMat);
		floorPlane.component<Transform>().scale = glm::vec3{ 2.0f, 2.0f, 2.0f };

		auto teapot = scene.createEntity("Teapot");
		teapot.addComponent<Mesh>(teapotMesh);
		teapot.addComponent<Graphics::DefaultMaterial>(metalMat);
		teapot.addComponent<Rotator>();
		teapot.component<Transform>().scale = glm::vec3{ 2.0f, 2.0f, 2.0f };

		// LIGHTS
		constexpr int lightCount = 64;
		constexpr float lightRadius = 10.0f;
		for (int i = 0; i < lightCount; i++)
		{
			float x = lightRadius * cos(glm::radians(360.0f / lightCount * i));
			float y = lightRadius * sin(glm::radians(360.0f / lightCount * i));
			auto light = scene.createEntity("Light " + std::to_string(i), { x, y, 3.0f });

			float r = Random::uniformFloat(0.0f, 1.0f);
			float g = Random::uniformFloat(0.0f, 1.0f);
			float b = Random::uniformFloat(0.0f, 1.0f);
			light.addComponent<PointLight>(glm::vec3{ r, g, b }, 200.0f);
		}
	}
};

auto main() -> int
{
	auto& engine = Aegix::Engine::instance();
	engine.loadScene<SimpleScene>();
	engine.run();
}
