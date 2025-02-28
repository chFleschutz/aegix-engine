#pragma once

#include "ui/layer.h"

#include "imgui.h"

namespace Aegix::UI 
{
	class RendererLayer : public Layer
	{
	public:
		virtual void onGuiRender() override
		{
			if (!ImGui::Begin("Renderer"))
			{
				ImGui::End();
				return;
			}

			ImGui::Text("Renderer Layer :)");


			ImGui::End();
		}
	};
}