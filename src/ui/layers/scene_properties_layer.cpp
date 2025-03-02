#include "scene_properties_layer.h"

#include "core/engine.h"
#include "graphics/systems/default_render_system.h"
#include "scene/components.h"

#include "imgui_internal.h"
#include "imgui_stdlib.h"

namespace Aegix::UI
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
		for (auto entity : scene.registry().view<entt::entity>())
		{
			drawEntityNode(Scene::Entity{ entity, &scene });
		}

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

		drawComponent<Name>("Name", m_selectedEntity, [](Name& nameComponent)
			{
				ImGui::InputText("Entity Name", &nameComponent.name);
			});

		drawComponent<Transform>("Transform", m_selectedEntity, [](Transform& transform)
			{
				ImGui::DragFloat3("Location", &transform.location.x, 0.1f);

				// Display rotation in degrees
				glm::vec3 rotationDeg = glm::degrees(transform.rotation);
				ImGui::DragFloat3("Rotation", &rotationDeg.x, 0.5f, 0.0f, 360.0f, "%.3f", ImGuiSliderFlags_WrapAround);
				transform.rotation = glm::radians(rotationDeg);

				ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);
			});

		drawComponent<Mesh>("Mesh", m_selectedEntity, [](Mesh& mesh)
			{
				drawAssetSlot("Mesh", "Mesh Asset", mesh.staticMesh != nullptr);
			});

		drawComponent<PointLight>("Pointlight", m_selectedEntity, [](PointLight& pointlight)
			{
				ImGui::ColorEdit3("Color", &pointlight.color.r);
				ImGui::DragFloat("Intensity", &pointlight.intensity, 0.1f);
			});

		drawComponent<Camera>("Camera", m_selectedEntity, [](Camera& camera)
			{
				ImGui::Text("Camera Component");
			});

		drawAddComponent();

		ImGui::End();
	}

	void ScenePropertiesLayer::drawEntityNode(Scene::Entity entity)
	{
		const char* name = "Entity";
		if (entity.hasComponent<Name>())
			name = entity.component<Name>().name.c_str();

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // For leaf nodes
		flags |= (m_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0; // Draw Selection

		ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, name);

		if (ImGui::IsItemClicked())
			m_selectedEntity = entity;

		if (ImGui::BeginPopupContextItem())
		{
			m_selectedEntity = entity;
			if (ImGui::MenuItem("Destroy Selected Entity"))
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
			drawAddComponentItem<Name>("Name");
			drawAddComponentItem<Transform>("Transform");
			drawAddComponentItem<Mesh>("Mesh");
			drawAddComponentItem<PointLight>("Point Light");
			drawAddComponentItem<Camera>("Camera");
			drawAddComponentItem<Graphics::DefaultMaterial>("Default Material");
			ImGui::EndPopup();
		}
	}

	void ScenePropertiesLayer::drawAssetSlot(const char* buttonLabel, const char* description, bool assetSet)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
		if (!assetSet)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

		ImVec2 buttonSize = ImVec2(50, 50);
		if (ImGui::Button(assetSet ? buttonLabel : "None", buttonSize))
		{
		}

		if (!assetSet)
			ImGui::PopStyleColor();

		ImGui::SameLine();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (buttonSize.y - ImGui::GetTextLineHeightWithSpacing()) * 0.5f);
		ImGui::Text(description);
		ImGui::PopStyleVar();
	}
}
