#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	class RasterPass : public FrameGraphPass
	{
	public:
		RasterPass(VulkanDevice& device) : FrameGraphPass(device) {}

		virtual void create() override;

		virtual void execute(const FrameInfo& frameInfo) override;

	private:
		void createRenderArea(const std::vector<FrameGraphPass::ResourceBinding>& attachmentRessources);
		void createRenderPass(const std::vector<FrameGraphPass::ResourceBinding>& attachmentRessources);
		void createFramebuffer(const std::vector<FrameGraphPass::ResourceBinding>& attachmentRessources);
		void createClearValues(const std::vector<FrameGraphPass::ResourceBinding>& attachmentRessources);

		void beginRenderPass(VkCommandBuffer commandBuffer);
		void endRenderPass(VkCommandBuffer commandBuffer);

		std::vector<FrameGraphPass::ResourceBinding> findAttachments();

		VkExtent2D m_renderArea = { 0, 0 };
		VkRenderPass m_renderPass = VK_NULL_HANDLE;
		VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
		std::vector<VkClearValue> m_clearValues{};
	};
}
