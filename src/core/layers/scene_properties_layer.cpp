#include "scene_properties_layer.h"

#include "core/engine.h"
#include "scene/components.h"

#include "imgui_internal.h"
#include "imgui_stdlib.h"

namespace Aegix
{
	void ScenePropertiesLayer::onGuiRender()
	{
		drawEntityView();
		drawEntityProperties();

		// Docking
		auto dockSpaceID = ImGui::GetID("ScenePropertyLayer");
		if (ImGui::DockBuilderGetNode(dockSpaceID) == NULL)
		{
			ImGui::DockBuilderRemoveNode(dockSpaceID); // Clear out existing layout
			ImGui::DockBuilderAddNode(dockSpaceID);
			ImGui::DockBuilderSetNodeSize(dockSpaceID, ImVec2(400, 600));

			ImGuiID entityDockID = ImGui::DockBuilderSplitNode(dockSpaceID, ImGuiDir_Up, 0.50f, NULL, &dockSpaceID);
			ImGuiID propDockID = ImGui::DockBuilderSplitNode(dockSpaceID, ImGuiDir_Down, 0.50f, NULL, &dockSpaceID);

			ImGui::DockBuilderDockWindow("Entity View", dockSpaceID);
			ImGui::DockBuilderDockWindow("Properties", propDockID);
			ImGui::DockBuilderFinish(dockSpaceID);
		}
	}

	void ScenePropertiesLayer::drawEntityView()
	{
		if (!ImGui::Begin("Entity View"))
		{
			ImGui::End();
			return;
		}
		auto& scene = Engine::instance().scene();
		scene.registry().each([&](auto entity)
			{
				drawEntityNode(Scene::Entity(entity, &scene));
			});

		if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
			m_selectedEntity = {};

		// Right click on window to create entity
		if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create Entity"))
				m_selectedEntity = scene.createEntity("Empty Entity");

			ImGui::EndPopup();
		}
		ImGui::End();
	}

	void ScenePropertiesLayer::drawEntityProperties()
	{
		if (!ImGui::Begin("Properties") || !m_selectedEntity)
		{
			ImGui::End();
			return;
		}

		drawComponent<Component::Name>("Name", m_selectedEntity, [](Component::Name& nameComponent)
			{
				ImGui::InputText("Entity Name", &nameComponent.name);
			});

		drawComponent<Component::Transform>("Transform", m_selectedEntity, [](Component::Transform& transform)
			{
				ImGui::DragFloat3("Location", &transform.location.x, 0.1f);
				ImGui::DragFloat3("Rotation", &transform.rotation.x, 0.1f);
				ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);
			});

		drawComponent<Component::Mesh>("Mesh", m_selectedEntity, [](Component::Mesh& mesh)
			{
				// TODO: Display mesh somehow
				ImGui::Text("Mesh Component");
			});

		drawComponent<Component::PointLight>("Pointlight", m_selectedEntity, [](Component::PointLight& pointlight)
			{
				ImGui::ColorEdit3("Color", &pointlight.color.r);
				ImGui::DragFloat("Intensity", &pointlight.intensity, 0.1f);
			});

		drawComponent<Component::Camera>("Camera", m_selectedEntity, [](Component::Camera& camera)
			{
				ImGui::Text("Camera Component");
			});

		drawAddComponent();

		ImGui::End();
	}

	void ScenePropertiesLayer::drawEntityNode(Scene::Entity entity)
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
			m_selectedEntity = entity;
			if (ImGui::MenuItem("Delete Entity"))
			{
				m_selectedEntity = {};
				Engine::instance().scene().destroyEntity(entity);
			}

			ImGui::EndPopup();
		}
	}

	void ScenePropertiesLayer::drawAddComponent()
	{
		ImGui::Spacing();

		float addButtonWidth = 200.0f;
		float windowWidth = ImGui::GetWindowWidth();
		ImGui::SetCursorPosX((windowWidth - addButtonWidth) * 0.5f);
		if (ImGui::Button("Add Component", ImVec2(addButtonWidth, 0.0f)))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			drawAddComponentItem<Component::Name>("Name");
			drawAddComponentItem<Component::Transform>("Transform");
			drawAddComponentItem<Component::Mesh>("Mesh");
			drawAddComponentItem<Component::PointLight>("Point Light");
			drawAddComponentItem<Component::Camera>("Camera");
			ImGui::EndPopup();
		}
	}
}
