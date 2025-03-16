#include "pch.h"

#include "editor_layer.h"

#include "core/engine.h"
#include "math/math.h"

#include <ImGuizmo.h>

namespace Aegix::Core
{
	void EditorLayer::onUpdate(float deltaSeconds)
	{
		m_snapping = Input::instance().keyPressed(Input::LeftControl);

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

		float snapValue = 0.5f;
		if (m_gizmoType == ImGuizmo::OPERATION::SCALE)
			snapValue = 0.1f;
		if (m_gizmoType == ImGuizmo::OPERATION::ROTATE)
			snapValue = 15.0f;

		float snapValues[3] = { snapValue, snapValue, snapValue };

		ImGuizmo::Manipulate(glm::value_ptr(camera.viewMatrix), glm::value_ptr(projectionMatrix),
			static_cast<ImGuizmo::OPERATION>(m_gizmoType), ImGuizmo::MODE::LOCAL, glm::value_ptr(transformMatrix),
			nullptr, m_snapping ? snapValues : nullptr);

		if (ImGuizmo::IsUsing())
		{
			Math::decomposeTRS(transformMatrix, transform.location, transform.rotation, transform.scale);
		}
	}
}
