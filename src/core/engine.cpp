#include "engine.h"

#include "core/editor_layer.h"
#include "core/profiler.h"

#include <chrono>

namespace Aegix
{
	Engine::Engine()
	{
		assert(s_instance == nullptr && "Only one instance of Engine is allowed");
		s_instance = this;

		m_layerStack.push<Core::EditorLayer>();
		
		ALOG::info("Engine Initialized!");
		Logging::logo();
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
			AGX_PROFILE_SCOPE("Frame Time");

			// Calculate time
			auto currentFrameBegin = std::chrono::high_resolution_clock::now();
			float frameTimeSec = std::chrono::duration<float, std::chrono::seconds::period>(currentFrameBegin - lastFrameBegin).count();
			lastFrameBegin = currentFrameBegin;

			glfwPollEvents();

			// Update 
			m_scene.update(frameTimeSec);
			m_layerStack.update(frameTimeSec);

			// Rendering
			m_renderer.renderFrame(m_scene, m_ui);

			applyFrameBrake(currentFrameBegin);
		}

		m_renderer.waitIdle();
	}

	void Engine::applyFrameBrake(std::chrono::high_resolution_clock::time_point lastFrameBegin)
	{
		AGX_PROFILE_SCOPE("Wait for FPS limit");

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
