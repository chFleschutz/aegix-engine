#include "pch.h"
#include "engine.h"

#include "core/editor_layer.h"
#include "core/profiler.h"
#include "core/globals.h"

namespace Aegis
{
	Engine::Engine()
	{
		AGX_ASSERT_X(!s_instance, "Only one instance of Engine is allowed");
		s_instance = this;

		m_assets.loadDefaultAssets();
		m_layerStack.push<Core::EditorLayer>();

		ALOG::info("Engine Initialized!");
		Logging::logo();
	}

	auto Engine::instance() -> Engine&
	{
		AGX_ASSERT_X(s_instance, "Engine instance not created yet");
		return *s_instance;
	}

	void Engine::run()
	{
		// Main Update loop
		auto lastFrameBegin = std::chrono::steady_clock::now();
		while (!m_window.shouldClose())
		{
			AGX_PROFILE_SCOPE("Frame Time");

			// Calculate time
			auto currentFrameBegin = std::chrono::steady_clock::now();
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

	void Engine::applyFrameBrake(std::chrono::steady_clock::time_point frameBegin)
	{
		using namespace std::chrono;

		AGX_PROFILE_SCOPE("Wait for FPS limit");

		if constexpr (!Core::ENABLE_FPS_LIMIT)
			return;

		// Note: This has to be a busy wait since sleep_for is not accurate enough (especially on Windows)
		while (duration<double, std::milli>(steady_clock::now() - frameBegin).count() < Core::TARGET_FRAME_TIME)
		{
			// busy wait
		}
	}
}
