#pragma once

#include "core/layers/layer.h"

#include "imgui.h"

namespace Aegix
{
	class DemoLayer : public Layer
	{
	public:
		virtual void onGuiRender() override
		{
			ImGui::ShowDemoWindow();
		}
	};
}