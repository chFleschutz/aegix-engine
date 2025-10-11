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


void createMyMaterial(Aegix::Scene::Scene& scene)
{
	using namespace Aegix::Graphics;
	using namespace Aegix;

	auto globalSetLayout = DescriptorSetLayout::Builder{}
		.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.build();

	auto materialSetLayout = DescriptorSetLayout::Builder{}
		.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build();

	auto pipeline = Pipeline::GraphicsBuilder{}
		.addDescriptorSetLayout(globalSetLayout)
		.addDescriptorSetLayout(materialSetLayout)
		.addPushConstantRange(VK_SHADER_STAGE_ALL_GRAPHICS, 128)
		.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, SHADER_DIR "default_geometry.vert.spv")
		.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "default_geometry.frag.spv")
		.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
		.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
		.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
		.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
		.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
		.setDepthAttachment(VK_FORMAT_D32_SFLOAT)
		.build();

	auto pbrMatTemplate = std::make_shared<MaterialTemplate>(std::move(pipeline), std::move(globalSetLayout), std::move(materialSetLayout));
	pbrMatTemplate->addParameter("albedo", MaterialParamType::Vec3, glm::vec3{ 1.0f, 1.0f, 1.0f });
	pbrMatTemplate->addParameter("emissive", MaterialParamType::Vec3, glm::vec3{ 0.0f, 0.0f, 0.0f });
	pbrMatTemplate->addParameter("metallic", MaterialParamType::Float, 0.0f);
	pbrMatTemplate->addParameter("roughness", MaterialParamType::Float, 1.0f);
	pbrMatTemplate->addParameter("ambientOcclusion", MaterialParamType::Float, 1.0f);
	pbrMatTemplate->addParameter("albedoMap", MaterialParamType::Texture2D, Texture::create(glm::vec4(1.0f), VK_FORMAT_R8G8B8A8_UNORM));
	pbrMatTemplate->addParameter("normalMap", MaterialParamType::Texture2D, Texture::create(glm::vec4{ 0.5f, 0.5f, 1.0f, 0.0f }, VK_FORMAT_R8G8B8A8_UNORM));
	pbrMatTemplate->addParameter("metalRoughnessMap", MaterialParamType::Texture2D, Texture::create(glm::vec4(1.0f), VK_FORMAT_R8G8B8A8_UNORM));
	pbrMatTemplate->addParameter("ambientOcclusionMap", MaterialParamType::Texture2D, Texture::create(glm::vec4(1.0f), VK_FORMAT_R8G8B8A8_UNORM));
	pbrMatTemplate->addParameter("emissiveMap", MaterialParamType::Texture2D, Texture::create(glm::vec4(0.0f), VK_FORMAT_R8G8B8A8_UNORM));

	auto myMatInstance = std::make_shared<MaterialInstance>(pbrMatTemplate);
	myMatInstance->setParameter("albedo", glm::vec3{ 1.0f, 0.0f, 0.0f });
	myMatInstance->setParameter("emissive", glm::vec3{ 10.0f, 10.0f, 10.0f });
	myMatInstance->setParameter("roughness", 0.5f);

	auto otherMatInstance = std::make_shared<MaterialInstance>(pbrMatTemplate);
	otherMatInstance->setParameter("albedo", glm::vec3{ 0.0f, 1.0f, 0.0f });

	auto myMesh = StaticMesh::create(ASSETS_DIR "Misc/cube.obj");

	auto entity = scene.createEntity("MyEntity", { -2.0f, 0.0f, 1.0f });
	entity.add<Mesh>(myMesh);
	entity.add<Material>(myMatInstance);

	auto entity2 = scene.createEntity("MyEntity2", { 2.0f, 0.0f, 1.0f });
	entity2.add<Mesh>(myMesh);
	entity2.add<Material>(otherMatInstance);
	entity2.add<ColorChanger>();

	for (const auto& [name, param] : pbrMatTemplate->parameters())
	{
		if (param.type == MaterialParamType::Texture2D)
			continue;

		ALOG::info("Param: {:20} Size {:8} Offset{:8}", name, param.size, param.offset);
	}
}

/// @brief Scene with two helmets
class HelmetScene : public Aegix::Scene::Description
{
public:
	void initialize(Aegix::Scene::Scene& scene) override
	{
		using namespace Aegix;

		// TODO: DEBUG
		createMyMaterial(scene);

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
