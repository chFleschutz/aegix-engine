#pragma once

#include "core/engine.h"
#include "core/layers/layer.h"
#include "scene/components.h"
#include "scene/scene.h"

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
			auto view = scene.viewEntities<Component::Name, Component::Transform>();
			for (auto&& [entity, name, transform] : view.each())
			{
				addEntityNode(name.name);
			}

			ImGui::End();
		}

	private:
		void addEntityNode(const std::string& name)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(4 / 7.0f, 0.6f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(4 / 7.0f, 0.7f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(4 / 7.0f, 0.8f, 0.8f));
			ImGui::Button(name.c_str());
			ImGui::PopStyleColor(3);
		}
	};
}