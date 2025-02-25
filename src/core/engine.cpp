#include "engine.h"

#include "core/input.h"
#include "graphics/layers/main_menu_layer.h"
#include "core/systems/camera_system.h"

#include <chrono>
#include <iostream>

namespace Aegix
{
	Engine::Engine()
	{
		assert(s_instance == nullptr && "Only one instance of Engine is allowed");
		s_instance = this;

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

		m_gui.pushLayer<Graphics::MainMenuLayer>();
		m_systems.add<CameraSystem>();
	}

	Engine::~Engine()
	{
		s_instance = nullptr;
	}

	Engine& Engine::instance()
	{
		assert(s_instance != nullptr && "Engine instance is not created");
		return *s_instance;
	}

	void Engine::run()
	{
		assert(m_scene != nullptr && "Cannot run engine without a scene");

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

			// Update 
			m_scene->update(frameTimeSec);
			m_systems.update(frameTimeSec, *m_scene);
			m_gui.update(frameTimeSec);

			// Rendering
			m_renderer.renderFrame(*m_scene, m_gui);

			applyFrameBrake(frameBeginTime);
		}

		m_renderer.waitIdle();
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
