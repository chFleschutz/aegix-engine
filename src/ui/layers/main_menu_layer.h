#pragma once

#include "core/engine.h"
#include "ui/layer.h"
#include "ui/layers/demo_layer.h"
#include "ui/layers/scene_properties_layer.h"

#include "imgui.h"

namespace Aegix::Graphics
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
					if (ImGui::MenuItem("Scene Properties", nullptr, m_scenePropertiesShown))
					{
						toggleLayer<ScenePropertiesLayer>(m_scenePropertiesShown);
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
		bool m_demoShown = false;
	};
}