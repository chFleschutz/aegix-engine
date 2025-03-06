#pragma once

#include "core/layer.h"
#include "ui/panels/demo_panel.h"
#include "ui/panels/menu_bar_panel.h"
#include "ui/panels/profiler_panel.h"
#include "ui/panels/renderer_panel.h"
#include "ui/panels/scene_panel.h"

namespace Aegix
{
	class EditorLayer : public Core::Layer
	{
	public:
		enum class DrawFlags
		{
			None = 1 << 0,
			Renderer = 1 << 1,
			Scene = 1 << 2,
			Profiler = 1 << 3,
			Demo = 1 << 4
		};

		EditorLayer() = default;

		virtual void onUIRender() override
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
		}

	private:
		UI::MenuBarPanel m_menuBarPanel{};
		UI::RendererPanel m_rendererPanel{};
		UI::ScenePanel m_scenePanel{};
		UI::ProfilerPanel m_profilerPanel{};
		UI::DemoPanel m_demoPanel{};
	};
}