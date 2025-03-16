#pragma once

#include "scene/entity.h"

#include <imgui.h>

namespace Aegix::UI
{
	class ScenePanel
	{
	public:
		[[nodiscard]] auto selectedEntity() const -> Scene::Entity { return m_selectedEntity; }
		
		void draw();

	private:
		void drawHierachy();
		void drawAllEntities();
		void drawSceneSettings();
		void drawEntityProperties();

		void drawSingleEntity(Scene::Entity entity);
		auto drawEntityTreeNode(Scene::Entity entity, ImGuiTreeNodeFlags flags) -> bool;
		void drawEntityActions();
		void drawAddComponent();

		static void drawAssetSlot(const char* assetName, const char* description, bool assetSet = true);

		template<typename T>
			requires OptionalComponent<T>
		void drawComponent(const char* componentName, Scene::Entity entity, ImGuiTreeNodeFlags flags, 
			std::function<void(T&)> drawFunc)
		{
			if (!entity.hasComponent<T>())
				return;

			bool keepComponent = true;
			if (ImGui::CollapsingHeader(componentName, &keepComponent, flags))
				drawFunc(entity.component<T>());

			if (!keepComponent)
				entity.removeComponent<T>();

			ImGui::Spacing();
			ImGui::Spacing();
		}

		template<typename T>
			requires RequiredComponent<T>
		void drawComponent(const char* componentName, Scene::Entity entity, ImGuiTreeNodeFlags flags,
			std::function<void(T&)> drawFunc)
		{
			assert(entity.hasComponent<T>() && "Entity does not have the required component");

			if (ImGui::CollapsingHeader(componentName, nullptr, flags))
				drawFunc(entity.component<T>());

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