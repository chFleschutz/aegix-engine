#pragma once

#include "core/engine.h"
#include "ui/layer.h"
#include "ui/layers/demo_layer.h"
#include "ui/layers/renderer_layer.h"
#include "ui/layers/scene_layer.h"

#include "imgui.h"

namespace Aegix::UI
{
	/// @brief Shows the main menu bar of the application
	class MainMenuLayer : public Layer
	{
	public:
		virtual void onGuiRender() override
		{
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("View"))
				{
					if (ImGui::MenuItem("Renderer", nullptr, m_rendererShown))
					{
						toggleLayer<RendererLayer>(m_rendererShown);
					}

					if (ImGui::MenuItem("Scene", nullptr, m_scenePropertiesShown))
					{
						toggleLayer<SceneLayer>(m_scenePropertiesShown);
					}

					if (ImGui::MenuItem("ImGui Demo", nullptr, m_demoShown))
					{
						toggleLayer<DemoLayer>(m_demoShown);
					}

					ImGui::EndMenu();
				}

				ImGui::EndMainMenuBar();
			}
		}

	private:
		template<typename T>
		void toggleLayer(bool& currentlyShown)
		{
			auto& ui = Engine::instance().ui();
			if (currentlyShown)
			{
				ui.popLayer<T>();
			}
			else
			{
				ui.pushLayerIfNotExist<T>();
			}
			currentlyShown = !currentlyShown;
		}

		bool m_scenePropertiesShown = false;
		bool m_rendererShown = false;
		bool m_demoShown = false;
	};
}