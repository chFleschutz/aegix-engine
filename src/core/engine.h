#pragma once

#include "core/asset_manager.h"
#include "core/input.h"
#include "core/systems/system_manager.h"
#include "graphics/device.h"
#include "graphics/globals.h"
#include "graphics/renderer.h"
#include "graphics/window.h"
#include "scene/description.h"
#include "scene/scene.h"
#include "ui/ui.h"

#include <memory>
#include <type_traits>

namespace Aegix
{
	class Engine
	{
	public:
		static constexpr int MAX_FPS = 144; // Max frames per second, set 0 to disable

		Engine();
		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;
		~Engine();

		Engine& operator=(const Engine&) = delete;
		Engine& operator=(Engine&&) = delete;

		/// @brief Returns the instance of the engine
		static Engine& instance();

		Graphics::Window& window() { return m_window; }
		Graphics::VulkanDevice& device() { return m_device; }
		Graphics::Renderer& renderer() { return m_renderer; }
		UI::UI& ui() { return m_ui; }
		AssetManager& assetManager() { return m_assetManager; }
		SystemManager& systemManager() { return m_systems; }
		Scene::Scene& scene() { return m_scene; }

		void run();

		/// @brief Creates a scene from a description
		/// @tparam T Description of the scene (Derived from Scene::Description)
		template<Scene::DescriptionDerived T>
		void loadScene()
		{
			// TODO: Clear old scene
			T description{};
			description.initialize(m_scene);
		}

	private:
		void applyFrameBrake(std::chrono::steady_clock::time_point frameBeginTime);

		inline static Engine* s_instance = nullptr;

		Graphics::Window m_window{ Graphics::DEFAULT_WIDTH, Graphics::DEFAULT_HEIGHT, "Aegix" };
		Graphics::VulkanDevice m_device{ m_window };
		Graphics::Renderer m_renderer{ m_window, m_device };
		UI::UI m_ui{ m_window, m_renderer };
		Input m_input{ m_window };
		AssetManager m_assetManager{ m_renderer };
		SystemManager m_systems;
		Scene::Scene m_scene;
	};
}
