#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/vulkan_tools.h"

namespace Aegix::Graphics
{
	class UIPass : public FrameGraphRenderPass
	{
	public:
		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override
		{
			m_final = builder.add({
				"Final", 
				FrameGraphResourceType::Reference,
				FrameGraphResourceUsage::ColorAttachment
				});

			return FrameGraphNodeCreateInfo{
				.name = "UI",
				.inputs = { m_final },
				.outputs = { m_final }
			};
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo, const RenderContext& ctx) override
		{
			auto& texture = resources.texture(m_final);
			auto attachment = Tools::renderingAttachmentInfo(texture, VK_ATTACHMENT_LOAD_OP_LOAD, {});

			VkExtent2D extent = frameInfo.swapChainExtent;
			VkRenderingInfo renderingInfo{};
			renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
			renderingInfo.renderArea = { 0, 0, extent.width, extent.height };
			renderingInfo.layerCount = 1;
			renderingInfo.colorAttachmentCount = 1;
			renderingInfo.pColorAttachments = &attachment;

			vkCmdBeginRendering(ctx.cmd, &renderingInfo);

			ctx.ui.render(ctx.cmd);

			vkCmdEndRendering(ctx.cmd);
		}

	private:
		FrameGraphResourceHandle m_final;
	};
}