#include "Scene.h"

#include "components.h"
#include "entity.h"
#include "scripting/movement/kinematic_movement_controller.h"

#include <cassert>

namespace Aegix::Scene
{
	std::shared_ptr<Graphics::Model> Scene::loadModel(const std::filesystem::path& modelPath)
	{
		return Graphics::Model::createModelFromFile(m_device, modelPath);
	}

	Scene::Scene(Graphics::VulkanDevice& device) : m_device{ device }
	{
		auto camera = createEntity("Main Camera");
		camera.addComponent<Component::Camera>();
		camera.addComponent<Aegix::Scripting::KinematcMovementController>();
		auto& cameraTransform = camera.getComponent<Component::Transform>();
		cameraTransform.location = { 0.0f, -10.0f, -7.0f };
		cameraTransform.rotation = { -1.03f, 0.0f, 0.0f };
	}

	Entity Scene::camera()
	{
		auto group = m_registry.group<Component::Camera>();
		assert(group.size() == 1 && "Scene has to contain exactly one camera");
		return { group.front(), this };
	}

	Entity Scene::createEntity(const std::string& name, const Vector3& location)
	{
		Entity entity = { m_registry.create(), this };
		entity.addComponent<Component::Transform>(location);
		entity.addComponent<Component::Name>(name.empty() ? "Entity" : name);
		return entity;
	}
}
