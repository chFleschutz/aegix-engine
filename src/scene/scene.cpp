#include "Scene.h"

#include "camera.h"
#include "keyboard_movement_controller.h"

namespace vre
{
	Scene::Scene(VreDevice& device) : mDevice{device}, mCamera{createEntity()}
	{
		mCamera.addComponent<Camera>();
		mCamera.addComponent<KeyboardMovementController>();
		mCamera.transform.location = { -0.5f, 0.1f, -0.5 };
		mCamera.transform.rotation = { -0.4f, 0.8f, 0 };
	}

	SceneEntity& Scene::createEntity(const glm::vec3& location, const glm::vec3& rotation, const glm::vec3& scale)
	{
		auto entity = SceneEntity::createEmpty();
		entity.transform.location = location;
		entity.transform.rotation = rotation;
		entity.transform.scale = scale;
		auto emplaceResult = mObjects.emplace(entity.id(), std::move(entity));
		return emplaceResult.first->second;
	}

	SceneEntity& Scene::createEntity(std::shared_ptr<VreModel> model, const glm::vec3& location, const glm::vec3& rotation, const glm::vec3& scale)
	{
		auto object = SceneEntity::createModel(model);
		object.transform.location = location;
		object.transform.rotation = rotation;
		object.transform.scale = scale;
		auto emplaceResult = mObjects.emplace(object.id(), std::move(object));
		return emplaceResult.first->second;
	}

	SceneEntity& Scene::createPointLight(const glm::vec3& location, float intensity, float radius, const glm::vec3& color)
	{
		auto pointLight = SceneEntity::createPointLight(0.2f, radius, color);
		pointLight.transform.location = location;
		auto emplaceResult = mObjects.emplace(pointLight.id(), std::move(pointLight));
		return emplaceResult.first->second;
	}

	std::shared_ptr<VreModel> Scene::loadModel(const std::filesystem::path& modelPath)
	{
		return VreModel::createModelFromFile(mDevice, modelPath);
	}

} // namespace vre
