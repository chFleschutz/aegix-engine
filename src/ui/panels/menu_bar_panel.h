#pragma once

#include "core/engine.h"

#include <imgui.h>

namespace Aegix::UI
{
	/// @brief Shows the main menu bar of the application
	class MenuBarPanel
	{
	public:
		enum MenuFlags
		{
			None = 1 << 0,
			Renderer = 1 << 1,
			Scene = 1 << 2,
			Profiler = 1 << 3,
			Demo = 1 << 4
		};

		[[nodiscard]] auto flagActive(MenuFlags flag) const -> bool { return m_flags & flag; }

		void draw()
		{
			if (!ImGui::BeginMainMenuBar())
				return;

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Renderer", nullptr, m_flags & MenuFlags::Renderer))
					m_flags ^= MenuFlags::Renderer;

				if (ImGui::MenuItem("Scene", nullptr, m_flags & MenuFlags::Scene))
					m_flags ^= MenuFlags::Scene;

				if (ImGui::MenuItem("Profiler", nullptr, m_flags & MenuFlags::Profiler))
					m_flags ^= MenuFlags::Profiler;

				if (ImGui::MenuItem("ImGui Demo", nullptr, m_flags & MenuFlags::Demo))
					m_flags ^= MenuFlags::Demo;

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

	private:
		uint32_t m_flags = MenuFlags::None;
	};
}