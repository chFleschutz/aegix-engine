#include "Scene.h"

#include "components.h"
#include "entity.h"
#include "scripting/movement/kinematic_movement_controller.h"

#include <cassert>

namespace VEScene
{
	std::shared_ptr<VEGraphics::Model> Scene::loadModel(const std::filesystem::path& modelPath)
	{
		return VEGraphics::Model::createModelFromFile(m_device, modelPath);
	}

	Scene::Scene(VEGraphics::VulkanDevice& device) : m_device{ device }
	{
		auto camera = createEntity("Main Camera");
		camera.addComponent<VEComponent::Camera>();
		camera.addScript<VEScripting::KinematcMovementController>();
		auto& cameraTransform = camera.getComponent<VEComponent::Transform>();
		cameraTransform.location = { 0.0f, -15.0f, -15.0f };
		cameraTransform.rotation = { -0.9f, 0.0f, 0.0f };
	}

	Entity Scene::camera()
	{
		auto group = m_registry.group<VEComponent::Camera>();
		assert(group.size() == 1 && "Scene has to contain exactly one camera");
		return { group.front(), this };
	}

	void Scene::runtimeBegin()
	{
		auto view = m_registry.view<VEComponent::Script>();
		// Initialize all script components
		for (auto&& [entity, component] : view.each())
		{
			component.script->m_entity = { entity, this };
		}

		// Call begin after all scripts have been initialized
		for (auto&& [entity, component] : view.each())
		{
			component.script->begin();
		}
	}

	void Scene::update(float deltaSeconds)
	{
		for (auto&& [entity, component] : m_registry.view<VEComponent::Script>().each())
		{
			component.script->update(deltaSeconds);
		}
	}

	void Scene::runtimeEnd()
	{
		for (auto&& [entity, component] : m_registry.view<VEComponent::Script>().each())
		{
			component.script->end();
		}
	}

	Entity Scene::createEntity(const std::string& name, const Vector3& location)
	{
		Entity entity = { m_registry.create(), this };
		entity.addComponent<VEComponent::Transform>(location);
		entity.addComponent<VEComponent::Name>(name.empty() ? "Entity" : name);
		return entity;
	}

} // namespace VEScene
