#include "default_scene.h"

namespace vre
{
	DefaultScene::DefaultScene(VreDevice& device) : Scene(device)
	{
		{ // Models
			auto teapot = loadModel("models/teapot.obj");
			createEntity(teapot, { -0.75f, 0.5f, 0.0f });

			auto plane = loadModel("models/plane.obj");
			createEntity(plane, { 0.0f, 0.5f, 0.0f });
		}
		{ // Lights
			createPointLight({ -1.0f, -1.0f, -1.0f });
			createPointLight({ 0.0f, -1.0f, -1.0f });
		}
	}

} // namespace vre
