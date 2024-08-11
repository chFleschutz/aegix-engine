#include "engine.h"

#include "core/input.h"
#include "graphics/buffer.h"
#include "graphics/systems/point_light_system.h"
#include "graphics/systems/simple_render_system.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "scripting/script_base.h"
#include "utils/math_utils.h"

#include <array>
#include <chrono>
#include <iostream>
#include <stdexcept>

namespace Aegix
{
	Engine::Engine()
	{
		std::cout << "Engine Initialized!\n";

		std::cout <<
			"\n\n"
			"\t\t\t\t      ###  ########  ######   ##  ##     ##\n"
			"\t\t\t\t     ## ## ##       ##    ##  ##   ##   ## \n"
			"\t\t\t\t    ##  ## ##       ##        ##    ## ##  \n"
			"\t\t\t\t   ##   ## ######   ##        ##     ###   \n"
			"\t\t\t\t  ######## ##       ##  ####  ##    ## ##  \n"
			"\t\t\t\t ##     ## ##       ##    ##  ##   ##   ## \n"
			"\t\t\t\t##      ## ########  ######   ##  ##     ##\n"
			"\n\n";
	}

	void Engine::run()
	{
		// Init Input
		Input::instance().initialize(m_window.glfwWindow());

		// Init Scene
		m_scene->initialize();

		// Init Camera
		auto& camera = m_scene->camera().getComponent<Aegix::Component::Camera>().camera;
		auto& cameraTransform = m_scene->camera().getComponent<Aegix::Component::Transform>();

		auto currentTime = std::chrono::high_resolution_clock::now();

		// ***********
		// update loop
		while (!m_window.shouldClose())
		{
			// Calculate time
			auto frameBeginTime = std::chrono::high_resolution_clock::now();
			float frameTimeSec = std::chrono::duration<float, std::chrono::seconds::period>(frameBeginTime - currentTime).count();
			currentTime = frameBeginTime;

			glfwPollEvents();

			// Update all components
			m_scene->update(frameTimeSec);

			float aspect = m_renderer.aspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 100.0f);
			camera.setViewYXZ(cameraTransform.location, cameraTransform.rotation);

			// RENDERING
			m_renderer.renderFrame(frameTimeSec, *m_scene, camera);

			applyFrameBrake(frameBeginTime);
		}

		m_renderer.shutdown();
		m_scene->runtimeEnd();
	}

	void Engine::applyFrameBrake(std::chrono::steady_clock::time_point frameBeginTime)
	{
		// FPS
		//std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - frameBeginTime) << std::endl;

		if (MAX_FPS <= 0)
			return;

		auto desiredFrameTime = std::chrono::round<std::chrono::nanoseconds>(std::chrono::duration<float>(1.0f / MAX_FPS));
		while (std::chrono::high_resolution_clock::now() < frameBeginTime + desiredFrameTime);
	}
}
