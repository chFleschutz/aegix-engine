#pragma once

#include "graphics/renderer.h"
#include "graphics/window.h"

#include "imgui.h"

namespace Aegix::Graphics
{
	class GUI
	{
	public:
		GUI(Window& window, Renderer& renderer);
		~GUI();

		void beginRender();
		void endRender(VkCommandBuffer commandBuffer);

	private:

	};
}