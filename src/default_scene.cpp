#include "default_scene.h"

namespace vre
{
	DefaultScene::DefaultScene(VreDevice& device) : Scene(device)
	{
		{ // Models
			auto teapot = loadModel("models/teapot.obj");
			create(teapot, { -0.75f, 0.5f, 0.0f });

			auto plane = loadModel("models/plane.obj");
			create(plane, { 0.0f, 0.5f, 0.0f });
		}
		{ // Lights
			createPointLight(0.2f, { -1.0f, -1.0f, -1.0f });
			createPointLight(0.2f, { 0.0f, -1.0f, -1.0f });
		}
	}

} // namespace vre
