#include "Scene.h"

#include "scene/components.h"
#include "scene/entity.h"
#include "scripting/movement/kinematic_movement_controller.h"

#include <cassert>

namespace Aegix::Scene
{
	Scene::Scene() 
	{
		auto camera = createEntity("Main Camera");
		camera.addComponent<Camera>();
		camera.addComponent<Scripting::KinematcMovementController>();
		auto& cameraTransform = camera.component<Transform>();
		cameraTransform.location = { 0.0f, -15.0f, 10.0f };
		cameraTransform.rotation = { glm::radians(-30.0f), 0.0f, 0.0f};
	}

	Entity Scene::camera()
	{
		auto group = m_registry.group<Camera>();
		assert(group.size() == 1 && "Scene has to contain exactly one camera");
		return { group.front(), this };
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
}
