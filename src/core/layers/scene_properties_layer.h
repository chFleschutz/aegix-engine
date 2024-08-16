#pragma once

#include "core/engine.h"
#include "core/layers/layer.h"
#include "scene/components.h"
#include "scene/scene.h"
#include "scene/entity.h"

#include "imgui.h"

#include <functional>

namespace Aegix
{
	class ScenePropertiesLayer : public Layer
	{
	public:
		virtual void onGuiRender() override;

	private:
		void drawEntityView();
		void drawEntityProperties();
		void drawEntityNode(Scene::Entity entity);
		void drawAddComponent();

		template<typename T>
		void drawComponent(const char* componentName, Scene::Entity entity, std::function<void(T&)> drawFunc)
		{
			if (!entity.hasComponent<T>())
				return;

			if (ImGui::CollapsingHeader(componentName))
				drawFunc(entity.getComponent<T>());
		
			ImGui::Spacing();
			ImGui::Spacing();
		}

		template<typename T>
		void drawAddComponentItem(const std::string& itemName)
		{
			if (m_selectedEntity.hasComponent<T>())
				return;

			if (ImGui::MenuItem(itemName.c_str()))
			{
				m_selectedEntity.addComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}

		Scene::Entity m_selectedEntity{};
	};
}