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
		Input::instance().initialize(m_window.glfwWindow());

		m_scene->initialize();

		auto currentTime = std::chrono::high_resolution_clock::now();

		// Main Update loop
		while (!m_window.shouldClose())
		{
			// Calculate time
			auto frameBeginTime = std::chrono::high_resolution_clock::now();
			float frameTimeSec = std::chrono::duration<float, std::chrono::seconds::period>(frameBeginTime - currentTime).count();
			currentTime = frameBeginTime;

			glfwPollEvents();

			// Update all components
			m_scene->update(frameTimeSec);

			// Rendering
			m_renderer.renderFrame(frameTimeSec, *m_scene);

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
