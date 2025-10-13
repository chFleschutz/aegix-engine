#include <aegix/engine.h>
#include <aegix/math/random.h>
#include <aegix/scene/description.h>
#include <aegix/scripting/script_base.h>

/// @brief Example script to rotate the entity around the up axis
class Rotator : public Aegix::Scripting::ScriptBase
{
protected:
	void update(float deltaSeconds) override
	{
		auto& transform = get<Aegix::Transform>();
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

		// MODELS
		auto teapotMesh = Graphics::StaticMesh::create(ASSETS_DIR "Misc/teapot.obj");
		auto planeMesh = Graphics::StaticMesh::create(ASSETS_DIR "Misc/plane.obj");

		// MATERIALS
		auto paintingTexture = Graphics::Texture::create(ASSETS_DIR "Misc/painting.png", VK_FORMAT_R8G8B8A8_SRGB);
		auto metalTexture = Graphics::Texture::create(ASSETS_DIR "Misc/brushed-metal.png", VK_FORMAT_R8G8B8A8_SRGB);
		
		auto pbrMatTemplate = Engine::assets().get<Graphics::MaterialTemplate>("default/PBR_template");

		auto paintingMat = Graphics::MaterialInstance::create(pbrMatTemplate);
		paintingMat->setParameter("albedoMap", paintingTexture);

		auto metalMat = Graphics::MaterialInstance::create(pbrMatTemplate);
		metalMat->setParameter("albedoMap", metalTexture);
		metalMat->setParameter("metallic", 1.0f);
		
		// ENTITIES
		auto floorPlane = scene.createEntity("Floor Plane");
		floorPlane.get<Transform>().scale = glm::vec3{ 2.0f, 2.0f, 2.0f };
		floorPlane.add<Mesh>(planeMesh);
		floorPlane.add<Material>(paintingMat);

		auto teapot = scene.createEntity("Teapot");
		teapot.get<Transform>().scale = glm::vec3{ 2.0f, 2.0f, 2.0f };
		teapot.add<Mesh>(teapotMesh);
		teapot.add<Material>(metalMat);
		teapot.add<Rotator>();

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
			light.add<PointLight>(glm::vec3{ r, g, b }, 200.0f);
		}
	}
};

auto main() -> int
{
	auto& engine = Aegix::Engine::instance();
	engine.loadScene<SimpleScene>();
	engine.run();
}
