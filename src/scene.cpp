#include "Scene.h"

namespace vre
{
	Scene::Scene(VreDevice& device) : mDevice{device}
	{
	}

	VreSceneObject& Scene::create(std::shared_ptr<VreModel> model, const glm::vec3& location)
	{
		auto object = VreSceneObject::createModel(model);
		object.transform.location = location;
		object.transform.scale = glm::vec3{ 3.0f };
		auto emplaceResult = mObjects.emplace(object.id(), std::move(object));
		return emplaceResult.first->second;
	}

	VreSceneObject& Scene::createPointLight(float intensity, const glm::vec3& location)
	{
		auto pointLight = VreSceneObject::createPointLight(0.2f);
		pointLight.transform.location = { -1.0f, -1.0f, -1.0f };
		auto emplaceResult = mObjects.emplace(pointLight.id(), std::move(pointLight));
		return emplaceResult.first->second;
	}

	std::shared_ptr<VreModel> Scene::loadModel(const std::filesystem::path& modelPath)
	{
		return VreModel::createModelFromFile(mDevice, modelPath);
	}

} // namespace vre
