#pragma once

#include "scene/entity.h"
#include "ui/layer.h"

#include "imgui.h"

#include <functional>

namespace Aegix::UI
{
	class SceneLayer : public Layer
	{
	public:
		virtual void onGuiRender() override;

	private:
		void drawAllEntities();
		void drawSceneSettings();
		void drawEntityProperties();
		void drawEntityNode(Scene::Entity entity, bool destroyPopup = true);
		void drawAddComponent();
		static void drawAssetSlot(const char* assetName, const char* description, bool assetSet = true);

		template<typename T>
		void drawComponent(const char* componentName, Scene::Entity entity, std::function<void(T&)> drawFunc)
		{
			if (!entity.hasComponent<T>())
				return;

			bool keepComponent = true;
			if (ImGui::CollapsingHeader(componentName, &keepComponent, ImGuiTreeNodeFlags_DefaultOpen))
				drawFunc(entity.component<T>());

			if (!keepComponent)
				entity.removeComponent<T>();

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