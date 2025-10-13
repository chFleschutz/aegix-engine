#pragma once

#include "core/asset_manager.h"
#include "core/globals.h"
#include "core/input.h"
#include "core/layer_stack.h"
#include "core/logging.h"
#include "graphics/graphics.h"
#include "scene/description.h"
#include "scene/scene.h"
#include "ui/ui.h"

namespace Aegix
{
	class Engine
	{
	public:
		Engine();
		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;
		~Engine() = default;

		auto operator=(const Engine&) -> Engine& = delete;
		auto operator=(Engine&&) -> Engine& = delete;

		/// @brief Returns the instance of the engine
		static Engine& instance();

		[[nodiscard]] auto window() -> Core::Window& { return m_window; }
		[[nodiscard]] auto renderer() -> Graphics::Renderer& { return m_graphics.renderer(); }
		[[nodiscard]] auto ui() -> UI::UI& { return m_ui; }
		[[nodiscard]] auto scene() -> Scene::Scene& { return m_scene; }

		[[nodiscard]] static auto assets() -> Core::AssetManager& { return Engine::instance().m_assets; }

		void run();

		/// @brief Creates a scene from a description
		/// @tparam T Description of the scene (Derived from Scene::Description)
		template<Scene::DescriptionDerived T>
		void loadScene()
		{
			m_scene.reset();
			T description{};
			description.initialize(m_scene);
		}

	private:
		void applyFrameBrake(std::chrono::steady_clock::time_point frameBeginTime);

		Logging m_logging{};
		Core::AssetManager m_assets{};
		Core::LayerStack m_layerStack{};
		Core::Window m_window{ Core::DEFAULT_WIDTH,Core::DEFAULT_HEIGHT, "Aegix" };
		Graphics::Graphics m_graphics{ m_window };
		UI::UI m_ui{ m_graphics, m_layerStack};
		Input m_input{ m_window };
		Scene::Scene m_scene;
	};
}
