#include "Scene.h"

#include "components.h"
#include "entity.h"
#include "keyboard_movement_controller.h"

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
		camera.addComponent<VEComponents::CameraComponent>();
		camera.addScript<VEScripting::KeyboardMovementController>();
		auto& cameraTransform = camera.getComponent<VEComponents::TransformComponent>();
		cameraTransform.location = { 0.0f, -15.0f, -15.0f };
		cameraTransform.rotation = { -0.9f, 0.0f, 0.0f };
	}

	Entity Scene::camera()
	{
		auto group = m_registry.group<VEComponents::CameraComponent>();
		assert(group.size() == 1 && "Scene has to contain exactly one camera");
		return { group.front(), this };
	}

	void Scene::runtimeBegin()
	{
		auto view = m_registry.view<VEComponents::ScriptComponent>();
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
		for (auto&& [entity, component] : m_registry.view<VEComponents::ScriptComponent>().each())
		{
			component.script->update(deltaSeconds);
		}
	}

	void Scene::runtimeEnd()
	{
		for (auto&& [entity, component] : m_registry.view<VEComponents::ScriptComponent>().each())
		{
			component.script->end();
		}
	}

	Entity Scene::createEntity(const std::string& name, const Vector3& location)
	{
		Entity entity = { m_registry.create(), this };
		entity.addComponent<VEComponents::TransformComponent>(location);
		entity.addComponent<VEComponents::NameComponent>(name.empty() ? "Entity" : name);
		return entity;
	}

} // namespace vre
