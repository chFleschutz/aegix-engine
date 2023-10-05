#include "Scene.h"

namespace vre
{
	Scene::Scene(VreDevice& device) : mDevice{device}
	{
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
