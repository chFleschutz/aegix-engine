#include "Scene.h"

#include "core/engine.h"
#include "graphics/static_mesh.h"
#include "graphics/systems/default_render_system.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "scene/systems/camera_system.h"
#include "scripting/movement/kinematic_movement_controller.h"
#include "utils/math_utils.h"
#include "scene/gltf_loader.h"

#include <gltf.h>
#include <gltf_utils.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <chrono>

namespace Aegix::Scene
{
	Scene::Scene()
	{
		addSystem<CameraSystem>();
		
		m_mainCamera = createEntity("Main Camera");
		m_mainCamera.addComponent<Camera>();
		m_mainCamera.addComponent<Scripting::KinematcMovementController>();
		m_mainCamera.component<Transform>() = Transform{
			.location = { 0.0f, -15.0f, 10.0f },
			.rotation = { glm::radians(-30.0f), 0.0f, 0.0f}
		};

		m_ambientLight = createEntity("Ambient Light");
		m_ambientLight.addComponent<AmbientLight>();

		m_directionalLight = createEntity("Directional Light");
		m_directionalLight.addComponent<DirectionalLight>();
		m_directionalLight.component<Transform>().rotation = { glm::radians(60.0f), 0.0f, glm::radians(45.0f) };
	}

	auto Scene::createEntity(const std::string& name, const glm::vec3& location, const glm::vec3& rotation, 
		const glm::vec3& scale) -> Entity
	{
		Entity entity = { m_registry.create(), this };
		entity.addComponent<Transform>(location);
		entity.addComponent<Name>(name.empty() ? "Entity" : name);
		entity.addComponent<Parent>();
		return entity;
	}

	void Scene::destroyEntity(Entity entity)
	{
		// TODO: Destroy all scripts attached to the entity
		m_registry.destroy(entity);
	}

	void Scene::update(float deltaSeconds)
	{
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
			assert(false && "Unsupported file format");
		}

		return Entity{};
	}
}
