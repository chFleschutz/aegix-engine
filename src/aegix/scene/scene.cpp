#include "pch.h"
#include "Scene.h"

#include "core/profiler.h"
#include "engine.h"
#include "graphics/resources/static_mesh.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "scene/loader/gltf_loader.h"
#include "scene/loader/obj_loader.h"
#include "scene/systems/camera_system.h"
#include "scene/systems/transform_system.h"
#include "scripting/movement/kinematic_movement_controller.h"

namespace Aegix::Scene
{
	auto Scene::createEntity(const std::string& name, const glm::vec3& location, const glm::vec3& rotation,
		const glm::vec3& scale) -> Entity
	{
		Entity entity = { m_registry.create(), this };
		entity.add<Name>(name.empty() ? "Entity" : name);
		entity.add<Transform>(location, rotation, scale);
		entity.add<GlobalTransform>();
		entity.add<Parent>();
		entity.add<Siblings>();
		entity.add<Children>();
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
			OBJLoader loader{ *this, path };
			return loader.rootEntity();
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
		m_mainCamera.add<Camera>();
		m_mainCamera.add<Scripting::KinematcMovementController>();
		m_mainCamera.get<Transform>() = Transform{
			.location = { 0.0f, -15.0f, 10.0f },
			.rotation = glm::radians(glm::vec3{ -30.0f, 0.0f, 0.0f })
		};

		m_ambientLight = createEntity("Ambient Light");
		m_ambientLight.add<AmbientLight>();

		m_directionalLight = createEntity("Directional Light");
		m_directionalLight.add<DirectionalLight>();
		m_directionalLight.get<Transform>().rotation = glm::radians(glm::vec3{ 60.0f, 0.0f, 45.0f });

		m_skybox = createEntity("Skybox");
		auto& env = m_skybox.add<Environment>();
		env.skybox = Graphics::Texture::solidColorCube(glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
		env.irradiance = env.skybox;
		env.prefiltered = env.skybox;
		env.brdfLUT = Graphics::Texture::BRDFLUT();
	}
}
