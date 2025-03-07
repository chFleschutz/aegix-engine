#include "editor_layer.h"

#include "core/engine.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ImGuizmo.h>

namespace Aegix::Core
{
	void EditorLayer::onUpdate(float deltaSeconds)
	{
		if (!ImGuizmo::IsUsing())
		{
			if (Input::instance().keyPressed(Input::Q))
				m_gizmoType = -1;
			else if (Input::instance().keyPressed(Input::W))
				m_gizmoType = ImGuizmo::OPERATION::TRANSLATE;
			else if (Input::instance().keyPressed(Input::E))
				m_gizmoType = ImGuizmo::OPERATION::ROTATE;
			else if (Input::instance().keyPressed(Input::R))
				m_gizmoType = ImGuizmo::OPERATION::SCALE;
		}
	}

	void EditorLayer::onUIRender()
	{
		m_menuBarPanel.draw();

		if (m_menuBarPanel.flagActive(UI::MenuBarPanel::Renderer))
			m_rendererPanel.draw();

		if (m_menuBarPanel.flagActive(UI::MenuBarPanel::Scene))
			m_scenePanel.draw();

		if (m_menuBarPanel.flagActive(UI::MenuBarPanel::Profiler))
			m_profilerPanel.draw();

		if (m_menuBarPanel.flagActive(UI::MenuBarPanel::Demo))
			m_demoPanel.draw();

		if (m_gizmoType != -1)
			drawGizmo();
	}

	void EditorLayer::drawGizmo()
	{
		auto selected = m_scenePanel.selectedEntity();
		if (!selected)
			return;

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		auto& scene = Engine::instance().scene();
		auto& camera = scene.mainCamera().component<Camera>();
		glm::mat4 projectionMatrix = camera.projectionMatrix;
		projectionMatrix[1][1] *= -1.0f; // Flip y-axis 

		auto& transform = selected.component<Transform>();
		glm::mat4 transformMatrix = transform.matrix();

		ImGuizmo::Manipulate(
			glm::value_ptr(camera.viewMatrix),
			glm::value_ptr(projectionMatrix),
			static_cast<ImGuizmo::OPERATION>(m_gizmoType),
			ImGuizmo::MODE::LOCAL,
			glm::value_ptr(transformMatrix));

		if (ImGuizmo::IsUsing())
		{
			MathLib::decomposeTRS(transformMatrix, transform.location, transform.rotation, transform.scale);
		}
	}
}
