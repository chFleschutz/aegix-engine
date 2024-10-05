#pragma once

#include "graphics/renderpasses/render_pass.h"

namespace Aegix::Graphics
{
	class UiPass : public RenderPass
	{
	public:
		UiPass(VulkanDevice& device, const Window& window, VkDescriptorPool globalPool, VkRenderPass renderpass);
		~UiPass();

		void render(FrameInfo& frameInfo) override;
	};
}