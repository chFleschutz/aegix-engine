#include "Scene.h"

#include "scene/components.h"
#include "scene/entity.h"
#include "scripting/movement/kinematic_movement_controller.h"

#include <cassert>

namespace Aegix::Scene
{
	Scene::Scene() 
	{
		m_mainCamera = createEntity("Main Camera");
		m_mainCamera.addComponent<Camera>();
		m_mainCamera.addComponent<Scripting::KinematcMovementController>();
		auto& cameraTransform = m_mainCamera.component<Transform>();
		cameraTransform.location = { 0.0f, -15.0f, 10.0f };
		cameraTransform.rotation = { glm::radians(-30.0f), 0.0f, 0.0f};
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
