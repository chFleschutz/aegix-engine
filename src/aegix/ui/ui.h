#pragma once

#include "core/layer_stack.h"
#include "graphics/vulkan/volk_include.h"

namespace Aegix::Graphics
{
	class Graphics;
}

namespace Aegix::UI
{
	/// @brief Manages all GUI Layers for displaying ImGui elements
	/// @note This class is a wrapper around ImGui
	class UI
	{
	public:
		UI(Graphics::Graphics& graphics, Core::LayerStack& layerStack);
		UI(const UI&) = delete;
		UI(UI&&) = delete;
		~UI();

		UI& operator=(const UI&) = delete;
		UI& operator=(UI&&) = delete;

		/// @brief Renders all GUI elements
		void render(VkCommandBuffer commandBuffer);

	private:
		Core::LayerStack& m_layerStack;
	};
}