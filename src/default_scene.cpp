#include "default_scene.h"

namespace vre
{
	DefaultScene::DefaultScene(VreDevice& device) : Scene(device)
	{
		{
			std::shared_ptr<VreModel> vreModel = VreModel::createModelFromFile(mDevice, "models/teapot.obj");
			auto flatVase = VreSceneObject::createModel(vreModel);
			flatVase.transform.location = { -0.75f, 0.5f, 0.0f };
			flatVase.transform.scale = glm::vec3{ 3.0f };
			mObjects.emplace(flatVase.id(), std::move(flatVase));

			vreModel = VreModel::createModelFromFile(mDevice, "models/plane.obj");
			auto floor = VreSceneObject::createModel(vreModel);
			floor.transform.location = { 0.0f, 0.5f, 0.0f };
			floor.transform.scale = glm::vec3{ 3.0f };
			mObjects.emplace(floor.id(), std::move(floor));
		}
		{
			auto pointLight = VreSceneObject::createPointLight(0.2f);
			pointLight.transform.location = { -1.0f, -1.0f, -1.0f };
			mObjects.emplace(pointLight.id(), std::move(pointLight));

			pointLight = VreSceneObject::createPointLight(0.2f);
			pointLight.transform.location = { 0.0f, -1.0f, -1.0f };
			mObjects.emplace(pointLight.id(), std::move(pointLight));
		}
	}

} // namespace vre
