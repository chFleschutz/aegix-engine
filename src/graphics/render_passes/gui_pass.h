#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	class GUIPass : public FrameGraphRenderPass
	{
	public:
		GUIPass()
		{
		}

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override
		{
			m_final = builder.add({
				"Final", 
				FrameGraphResourceType::Reference,
				FrameGraphResourceUsage::ColorAttachment
				});

			return FrameGraphNodeCreateInfo{
				.name = "GUI Pass",
				.inputs = { m_final },
				.outputs = { m_final }
			};
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
			VkCommandBuffer cmd = frameInfo.commandBuffer;

			auto& texture = resources.texture(m_final);
			auto attachment = Tools::renderingAttachmentInfo(texture, VK_ATTACHMENT_LOAD_OP_LOAD, {});

			VkExtent2D extent = frameInfo.swapChainExtent;
			VkRenderingInfo renderingInfo{};
			renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
			renderingInfo.renderArea = { 0, 0, extent.width, extent.height };
			renderingInfo.layerCount = 1;
			renderingInfo.colorAttachmentCount = 1;
			renderingInfo.pColorAttachments = &attachment;

			vkCmdBeginRendering(cmd, &renderingInfo);

			frameInfo.gui.render(cmd);

			vkCmdEndRendering(cmd);
		}

	private:
		FrameGraphResourceHandle m_final;
	};
}