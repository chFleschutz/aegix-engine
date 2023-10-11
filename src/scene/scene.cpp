#include "Scene.h"

#include "camera.h"
#include "components.h"
#include "entity.h"
#include "keyboard_movement_controller.h"

#include <cassert>

namespace vre
{
	std::shared_ptr<VreModel> Scene::loadModel(const std::filesystem::path& modelPath)
	{
		return VreModel::createModelFromFile(mDevice, modelPath);
	}

	Scene::Scene(VreDevice& device) : mDevice{ device }
	{
		auto camera = createEntity("Main Camera");
		camera.addComponent<CameraComponent>();
		//camera.addComponent<KeyboardMovementController>();
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


	Entity Scene::createEntity(const std::string& name, const glm::vec3& location)
	{
		Entity entity = { m_registry.create(), this };
		entity.addComponent<TransformComponent>(location);
		entity.addComponent<NameComponent>(name.empty() ? "Entity" : name);
		return entity;
	}

} // namespace vre
