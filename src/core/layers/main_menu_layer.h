#pragma once

#include "core/engine.h"
#include "core/layers/layer.h"
#include "core/layers/scene_properties_layer.h"
#include "core/layers/demo_layer.h"

#include "imgui.h"

namespace Aegix
{
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
			auto& layerStack = Engine::instance().layerStack();
			if (currentlyShown)
			{
				layerStack.pop<T>();
			}
			else
			{
				layerStack.pushIfNotExist<T>();
			}
			currentlyShown = !currentlyShown;
		}

		bool m_scenePropertiesShown = false;
		bool m_demoShown = false;
	};
}