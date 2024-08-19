#pragma once

#include "core/engine.h"
#include "graphics/layers/layer.h"
#include "graphics/layers/scene_properties_layer.h"
#include "graphics/layers/demo_layer.h"

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
			auto& gui = Engine::instance().gui();
			if (currentlyShown)
			{
				gui.popLayer<T>();
			}
			else
			{
				gui.pushLayerIfNotExist<T>();
			}
			currentlyShown = !currentlyShown;
		}

		bool m_scenePropertiesShown = false;
		bool m_demoShown = false;
	};
}