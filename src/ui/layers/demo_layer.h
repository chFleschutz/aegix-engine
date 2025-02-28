#pragma once

#include "ui/layer.h"

#include "imgui.h"

namespace Aegix::UI
{
	/// @brief A demo layer that shows the ImGui demo window
	class DemoLayer : public Layer
	{
	public:
		virtual void onGuiRender() override
		{
			ImGui::ShowDemoWindow();
		}
	};
}