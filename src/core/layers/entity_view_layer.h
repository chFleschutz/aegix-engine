#pragma once

#include "core/engine.h"
#include "core/layers/layer.h"
#include "scene/components.h"
#include "scene/scene.h"
#include "scene/entity.h"

#include "imgui.h"

namespace Aegix
{
	class EntityViewLayer : public Layer
	{
	public:
		virtual void onGuiRender() override
		{
			ImGui::Begin("Entity View");

			auto& scene = Engine::instance().scene();
			scene.registry().each([&](auto entity)
				{
					addEntityNode(Scene::Entity(entity, &scene));
				});

			if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
				m_selectedEntity = {};
			
			// Right click on window to create entity
			if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
			{
				if (ImGui::MenuItem("Create Entity"))
					scene.createEntity("Empty Entity");

				ImGui::EndPopup();
			}

			ImGui::End();
		}

	private:
		void addEntityNode(Scene::Entity entity)
		{
			auto& name = entity.getComponent<Component::Name>().name;

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // For leaf nodes
			flags |= (m_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0; // Draw Selection

			ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, name.c_str());

			if (ImGui::IsItemClicked())
				m_selectedEntity = entity;

			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Delete Entity"))
				{
					if (m_selectedEntity == entity)
						m_selectedEntity = {};

					Engine::instance().scene().destroyEntity(entity);
				}

				ImGui::EndPopup();
			}
		}

		Scene::Entity m_selectedEntity{};
	};
}