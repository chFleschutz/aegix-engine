#pragma once

#include "core/layer.h"
#include "ui/panels/demo_panel.h"
#include "ui/panels/menu_bar_panel.h"
#include "ui/panels/profiler_panel.h"
#include "ui/panels/renderer_panel.h"
#include "ui/panels/scene_panel.h"
#include "ui/panels/statistics_panel.h"

namespace Aegix::Core
{
	class EditorLayer : public Core::Layer
	{
	public:
		EditorLayer() = default;

		virtual void onUpdate(float deltaSeconds) override;
		virtual void onUIRender() override;
		
	private:
		void drawGizmo();

		UI::MenuBarPanel m_menuBarPanel{};
		UI::RendererPanel m_rendererPanel{};
		UI::ScenePanel m_scenePanel{};
		UI::StatisticsPanel m_statisticsPanel{};
		UI::ProfilerPanel m_profilerPanel{};
		UI::DemoPanel m_demoPanel{};
		int m_gizmoType = -1;
		bool m_snapping = false;
	};
}