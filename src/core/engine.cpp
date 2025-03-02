#include "engine.h"

#include "core/input.h"
#include "ui/layers/main_menu_layer.h"

#include <chrono>
#include <iostream>
#include <thread>

namespace Aegix
{
	Engine::Engine()
	{
		assert(s_instance == nullptr && "Only one instance of Engine is allowed");
		s_instance = this;

		std::cout << "Engine Initialized!\n"
			"\n\n"
			"\t\t\t\t      ###  ########  ######   ##  ##     ##\n"
			"\t\t\t\t     ## ## ##       ##    ##  ##   ##   ## \n"
			"\t\t\t\t    ##  ## ##       ##        ##    ## ##  \n"
			"\t\t\t\t   ##   ## ######   ##        ##     ###   \n"
			"\t\t\t\t  ######## ##       ##  ####  ##    ## ##  \n"
			"\t\t\t\t ##     ## ##       ##    ##  ##   ##   ## \n"
			"\t\t\t\t##      ## ########  ######   ##  ##     ##\n"
			"\n\n";

		m_ui.pushLayer<UI::MainMenuLayer>();
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
		auto lastFrameBegin = std::chrono::high_resolution_clock::now();

		// Main Update loop
		while (!m_window.shouldClose())
		{
			// Calculate time
			auto currentFrameBegin = std::chrono::high_resolution_clock::now();
			float frameTimeSec = std::chrono::duration<float, std::chrono::seconds::period>(currentFrameBegin - lastFrameBegin).count();
			lastFrameBegin = currentFrameBegin;

			glfwPollEvents();

			// Update 
			m_scene.update(frameTimeSec);
			m_ui.update(frameTimeSec);

			// Rendering
			m_renderer.renderFrame(m_scene, m_ui);

			applyFrameBrake(currentFrameBegin);
		}

		m_renderer.waitIdle();
		m_scene.runtimeEnd();
	}

	void Engine::applyFrameBrake(std::chrono::high_resolution_clock::time_point lastFrameBegin)
	{
		if (MAX_FPS <= 0)
			return;

		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		std::chrono::milliseconds frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFrameBegin);

		static constexpr auto targetFrameTime = std::chrono::milliseconds(1000 / MAX_FPS);
		if (frameTime < targetFrameTime)
		{
			std::this_thread::sleep_for(targetFrameTime - frameTime);
		}
	}
}
