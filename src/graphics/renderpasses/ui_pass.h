#pragma once

#include "graphics/renderpasses/render_pass.h"

namespace Aegix::Graphics
{
	class UiPass : public RenderPass
	{
	public:
		UiPass(const Window& window, VulkanDevice& device, VkDescriptorPool globalPool, VkRenderPass renderpass);
		~UiPass();

		void render(FrameInfo& frameInfo) override;
	};
}