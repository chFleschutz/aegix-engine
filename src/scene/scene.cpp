#include "Scene.h"

#include "scene/components.h"
#include "scene/entity.h"
#include "scripting/movement/kinematic_movement_controller.h"
#include "scene/systems/camera_system.h"

namespace Aegix::Scene
{
	Scene::Scene()
	{
		m_mainCamera = createEntity("Main Camera");
		m_mainCamera.addComponent<Camera>();
		m_mainCamera.addComponent<Scripting::KinematcMovementController>();
		m_mainCamera.component<Transform>() = Transform{
			.location = { 0.0f, -15.0f, 10.0f },
			.rotation = { glm::radians(-30.0f), 0.0f, 0.0f}
		};

		addSystem<CameraSystem>();
	}

	Entity Scene::createEntity(const std::string& name, const glm::vec3& location)
	{
		Entity entity = { m_registry.create(), this };
		entity.addComponent<Transform>(location);
		entity.addComponent<Name>(name.empty() ? "Entity" : name);
		return entity;
	}

	void Scene::destroyEntity(Entity entity)
	{
		// TODO: Destroy all scripts attached to the entity
		// TODO: Destroy mesh after rendering finished and its not used anymore
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
}
