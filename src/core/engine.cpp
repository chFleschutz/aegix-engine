#include "engine.h"

#include "core/editor_layer.h"
#include "core/profiler.h"
#include "core/globals.h"

#include <chrono>

namespace Aegix
{
	Engine::Engine()
	{
		m_layerStack.push<Core::EditorLayer>();
		
		ALOG::info("Engine Initialized!");
		Logging::logo();
	}

	Engine& Engine::instance()
	{
		static Engine instance;
		return instance;
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
			m_graphics.renderer().renderFrame(m_scene, m_ui);

			applyFrameBrake(currentFrameBegin);
		}

		m_graphics.renderer().waitIdle();
	}

	void Engine::applyFrameBrake(std::chrono::high_resolution_clock::time_point lastFrameBegin)
	{
		AGX_PROFILE_SCOPE("Wait for FPS limit");

		if constexpr (!Core::ENABLE_FPS_LIMIT)
			return;

		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		std::chrono::milliseconds frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFrameBegin);

		if (frameTime < Core::TARGET_FRAME_TIME)
		{
			std::this_thread::sleep_for(Core::TARGET_FRAME_TIME - frameTime);
		}
	}
}
