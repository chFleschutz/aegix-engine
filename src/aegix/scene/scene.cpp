#include "pch.h"

#include "Scene.h"

#include "engine.h"
#include "core/profiler.h"
#include "graphics/static_mesh.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "scene/gltf_loader.h"
#include "scene/systems/camera_system.h"
#include "scene/systems/transform_system.h"
#include "scripting/movement/kinematic_movement_controller.h"

namespace Aegix::Scene
{
	Scene::Scene()
	{
	}

	auto Scene::createEntity(const std::string& name, const glm::vec3& location, const glm::vec3& rotation,
		const glm::vec3& scale) -> Entity
	{
		Entity entity = { m_registry.create(), this };
		entity.addComponent<Name>(name.empty() ? "Entity" : name);
		entity.addComponent<Transform>(location, rotation, scale);
		entity.addComponent<GlobalTransform>();
		entity.addComponent<Parent>();
		entity.addComponent<Siblings>();
		entity.addComponent<Children>();
		return entity;
	}

	void Scene::destroyEntity(Entity entity)
	{
		// TODO: Destroy all scripts attached to the entity

		entity.removeParent();
		entity.removeChildren();

		m_registry.destroy(entity);
	}

	void Scene::update(float deltaSeconds)
	{
		AGX_PROFILE_FUNCTION();

		for (auto& system : m_systems)
		{
			system->onUpdate(deltaSeconds, *this);
		}

		m_scriptManager.update(deltaSeconds);
	}

	auto Scene::load(const std::filesystem::path& path) -> Entity
	{
		if (path.extension() == ".gltf" || path.extension() == ".glb")
		{
			GLTFLoader loader{ *this, path };
			return loader.rootEntity();
		}
		else if (path.extension() == ".obj")
		{
			auto entity = createEntity(path.stem().string());
			entity.addComponent<Mesh>(Graphics::StaticMesh::create(path));
			//entity.addComponent<Graphics::DefaultMaterial>(); // TODO: Add default material
			return entity;
		}
		else
		{
			AGX_ASSERT_X(false, "Unsupported file format");
		}

		return Entity{};
	}

	void Scene::reset()
	{
		// TODO: Clear old scene

		addSystem<CameraSystem>();
		addSystem<TransformSystem>();

		m_mainCamera = createEntity("Main Camera");
		m_mainCamera.addComponent<Camera>();
		m_mainCamera.addComponent<Scripting::KinematcMovementController>();
		m_mainCamera.component<Transform>() = Transform{
			.location = { 0.0f, -15.0f, 10.0f },
			.rotation = glm::radians(glm::vec3{ -30.0f, 0.0f, 0.0f })
		};

		m_ambientLight = createEntity("Ambient Light");
		m_ambientLight.addComponent<AmbientLight>();

		m_directionalLight = createEntity("Directional Light");
		m_directionalLight.addComponent<DirectionalLight>();
		m_directionalLight.component<Transform>().rotation = glm::radians(glm::vec3{ 60.0f, 0.0f, 45.0f });

		m_skybox = createEntity("Skybox");
		auto& env = m_skybox.addComponent<Environment>();
		env.skybox = std::make_shared<Graphics::Texture>(Engine::instance().device());
		env.skybox->createCube(1, 1, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
		env.skybox->image().fillSFLOAT(glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
		env.irradiance = std::make_shared<Graphics::Texture>(Engine::instance().device());
		env.irradiance->createCube(1, 1, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
		env.irradiance->image().fillSFLOAT(glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
		env.prefiltered = std::make_shared<Graphics::Texture>(Engine::instance().device());
		env.prefiltered->createCube(1, 1, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
		env.prefiltered->image().fillSFLOAT(glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
		env.brdfLUT = Graphics::Texture::createBRDFLUT();
	}
}
