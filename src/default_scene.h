#pragma once

#include "scene.h"

namespace vre
{
	class Rotator : public Component
	{
	public:
		using Component::Component;

		void update(float deltaSeconds) override
		{
			entity().transform.rotation += glm::vec3{0.0f, 1.0f, 0.0f} *deltaSeconds;
		}
	};

	class DefaultScene : public Scene
	{
	public:
		DefaultScene(VreDevice& device) : Scene(device)
		{
			{ // Models
				auto teapotModel = loadModel("models/teapot.obj");
				auto& teapot = createEntity(teapotModel, { 0.0f, 0.5f, 0.0f });
				teapot.addComponent<Rotator>();

				auto plane = loadModel("models/plane.obj");
				createEntity(plane, { 0.0f, 0.5f, 0.0f });
			}
			{ // Lights
				createPointLight({ -1.0f, -1.0f, -1.0f });
				createPointLight({ 0.0f, -1.0f, -1.0f });
			}
		}
	};

} // namespace vre
