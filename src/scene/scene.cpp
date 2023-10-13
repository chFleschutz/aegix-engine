#include "Scene.h"

#include "components.h"
#include "entity.h"
#include "keyboard_movement_controller.h"

#include <cassert>

namespace vre
{
	std::shared_ptr<Model> Scene::loadModel(const std::filesystem::path& modelPath)
	{
		return Model::createModelFromFile(mDevice, modelPath);
	}

	Scene::Scene(VulkanDevice& device) : mDevice{ device }
	{
		auto camera = createEntity("Main Camera");
		camera.addComponent<CameraComponent>();
		camera.addScript<KeyboardMovementController>();
		auto& cameraTransform = camera.getComponent<TransformComponent>();
		cameraTransform.Location = { -0.5f, -0.5f, -0.5 };
		cameraTransform.Rotation = { -0.4f, 0.8f, 0 };
	}

	Entity Scene::camera()
	{
		auto group = m_registry.group<CameraComponent>();
		assert(group.size() == 1 && "Scene has to contain exactly one camera");
		return { group.front(), this };
	}

	void Scene::runtimeBegin()
	{
		auto view = m_registry.view<ScriptComponent>();
		// Initialize all script components
		for (auto&& [entity, component] : view.each())
		{
			component.Script->m_Entity = { entity, this };
		}

		// Call begin after all scripts have been initialized
		for (auto&& [entity, component] : view.each())
		{
			component.Script->begin();
		}
	}

	void Scene::update(float deltaSeconds)
	{
		for (auto&& [entity, component] : m_registry.view<ScriptComponent>().each())
		{
			component.Script->update(deltaSeconds);
		}
	}

	void Scene::runtimeEnd()
	{
		for (auto&& [entity, component] : m_registry.view<ScriptComponent>().each())
		{
			component.Script->end();
		}
	}

	Entity Scene::createEntity(const std::string& name, const glm::vec3& location)
	{
		Entity entity = { m_registry.create(), this };
		entity.addComponent<TransformComponent>(location);
		entity.addComponent<NameComponent>(name.empty() ? "Entity" : name);
		return entity;
	}

} // namespace vre
