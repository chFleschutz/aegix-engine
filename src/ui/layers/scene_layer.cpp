#include "scene_layer.h"

#include "core/engine.h"
#include "graphics/systems/default_render_system.h"
#include "scene/components.h"

#include "imgui_internal.h"
#include "imgui_stdlib.h"

namespace Aegix::UI
{
	void SceneLayer::onGuiRender()
	{
		drawAllEntities();
		drawSceneSettings();
		drawEntityProperties();

		// Docking
		ImGuiID dockSpaceID = ImGui::GetID("SceneLayer");
		if (ImGui::DockBuilderGetNode(dockSpaceID) == NULL)
		{
			ImGui::DockBuilderRemoveNode(dockSpaceID); // Clear out existing layout
			ImGui::DockBuilderAddNode(dockSpaceID);
			ImGui::DockBuilderSetNodePos(dockSpaceID, ImVec2(10, 30));
			ImGui::DockBuilderSetNodeSize(dockSpaceID, ImVec2(400, 600));

			ImGuiID dockMainID = dockSpaceID;
			ImGuiID dockBottomID = ImGui::DockBuilderSplitNode(dockSpaceID, ImGuiDir_Down, 0.50f, NULL, &dockMainID);

			ImGui::DockBuilderDockWindow("All Entities", dockMainID);
			ImGui::DockBuilderDockWindow("Scene Settings", dockMainID);
			ImGui::DockBuilderDockWindow("Properties", dockBottomID);
			ImGui::DockBuilderFinish(dockSpaceID);
		}
	}

	void SceneLayer::drawAllEntities()
	{
		ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(400, 600));
		if (!ImGui::Begin("All Entities"))
		{
			ImGui::End();
			return;
		}

		auto& scene = Engine::instance().scene();
		auto view = scene.registry().view<entt::entity>();

		for (auto entt : view)
		{
			Scene::Entity entity{ entt, &scene };
			if (entity.hasComponent<Parent>() && entity.component<Parent>().entity)
				continue;

			drawEntity(entity);
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

	void SceneLayer::drawSceneSettings()
	{
		if (!ImGui::Begin("Scene Settings"))
		{
			ImGui::End();
			return;
		}

		auto& scene = Engine::instance().scene();
		drawSingleEntity(scene.mainCamera());
		drawSingleEntity(scene.ambientLight());
		drawSingleEntity(scene.directionalLight());

		if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
			m_selectedEntity = {};

		ImGui::End();
	}

	void SceneLayer::drawEntityProperties()
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

				glm::vec3 rotationDeg = glm::degrees(transform.rotation);
				ImGui::DragFloat3("Rotation", &rotationDeg.x, 0.5f, 0.0f, 360.0f, "%.3f", ImGuiSliderFlags_WrapAround);
				transform.rotation = glm::radians(rotationDeg);

				ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);
			});

		drawComponent<Mesh>("Mesh", m_selectedEntity, [](Mesh& mesh)
			{
				drawAssetSlot("Mesh", "Mesh Asset", mesh.staticMesh != nullptr);
			});

		drawComponent<AmbientLight>("Ambient Light", m_selectedEntity, [](AmbientLight& ambientLight)
			{
				ImGui::ColorEdit3("Color", &ambientLight.color.r);
				ImGui::DragFloat("Intensity", &ambientLight.intensity, 0.01f, 0.0f, 1.0f);
			});

		drawComponent<DirectionalLight>("Directional Light", m_selectedEntity, [](DirectionalLight& directionalLight)
			{
				ImGui::ColorEdit3("Color", &directionalLight.color.r);
				ImGui::DragFloat("Intensity", &directionalLight.intensity, 0.1f, 0.0f, 10.0f);
			});

		drawComponent<PointLight>("Pointlight", m_selectedEntity, [](PointLight& pointlight)
			{
				ImGui::ColorEdit3("Color", &pointlight.color.r);
				ImGui::DragFloat("Intensity", &pointlight.intensity, 0.1f, 0.0f, 1000.0f);
			});

		drawComponent<Camera>("Camera", m_selectedEntity, [](Camera& camera)
			{
				float fovDeg = glm::degrees(camera.fov);
				ImGui::DragFloat("FOV", &fovDeg, 0.1f, 0.0f, 180.0f);
				camera.fov = glm::radians(fovDeg);

				ImGui::DragFloat("Near", &camera.near, 0.01f, 0.0f, 100.0f);
				ImGui::DragFloat("Far", &camera.far, 0.1f, 0.0f, 1000.0f);
				ImGui::Text("Aspect Ratio: %.2f", camera.aspect);
			});

		drawAddComponent();

		ImGui::End();
	}

	void SceneLayer::drawEntity(Scene::Entity entity)
	{
		auto& children = entity.getOrAddComponent<Children>();

		auto name = entity.hasComponent<Name>() ? entity.component<Name>().name.c_str() : "Entity";
		auto flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		flags |= (children.count == 0) ? ImGuiTreeNodeFlags_Leaf : 0;
		flags |= (m_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0; // Highlight selection

		if (!ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, name))
			return;

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

		for (auto child : children)
		{
			drawEntity(child);
		}

		ImGui::TreePop();
	}

	void SceneLayer::drawSingleEntity(Scene::Entity entity)
	{
		auto name = entity.hasComponent<Name>() ? entity.component<Name>().name.c_str() : "Entity";
		auto flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf;
		flags |= (m_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0; // Highlight selection

		if (ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, name))
			ImGui::TreePop();

		if (ImGui::IsItemClicked())
			m_selectedEntity = entity;
	}

	void SceneLayer::drawAddComponent()
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
			drawAddComponentItem<AmbientLight>("Ambient Light");
			drawAddComponentItem<DirectionalLight>("Directional Light");
			drawAddComponentItem<PointLight>("Point Light");
			drawAddComponentItem<Camera>("Camera");
			drawAddComponentItem<Mesh>("Mesh");
			drawAddComponentItem<Graphics::DefaultMaterial>("Default Material");
			ImGui::EndPopup();
		}
	}

	void SceneLayer::drawAssetSlot(const char* buttonLabel, const char* description, bool assetSet)
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
