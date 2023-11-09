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
		camera.addComponent<VEScripting::KinematcMovementController>();
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
		for (auto& script : m_scripts)
		{
			script->begin();
		}
	}

	void Scene::update(float deltaSeconds)
	{
		for (auto& script : m_scripts)
		{
			script->update(deltaSeconds);
		}
	}

	void Scene::runtimeEnd()
	{
		for (auto& script : m_scripts)
		{
			script->end();
		}
	}

	void Scene::addScript(VEScripting::ScriptBase* script)
	{
		m_scripts.push_back(script);
	}

	Entity Scene::createEntity(const std::string& name, const Vector3& location)
	{
		Entity entity = { m_registry.create(), this };
		entity.addComponent<VEComponent::Transform>(location);
		entity.addComponent<VEComponent::Name>(name.empty() ? "Entity" : name);
		return entity;
	}

} // namespace VEScene
