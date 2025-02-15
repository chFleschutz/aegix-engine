#pragma once

#include "graphics/frame_graph/frame_graph.h"
#include "graphics/frame_graph/frame_graph_blackboard.h"
#include "graphics/systems/render_system.h"

namespace Aegix::Graphics
{
	// TODO: move somewhere else
	struct SwapChainData
	{
		VkImageView colorImageView;
		VkImageView depthImageView;
		VkExtent2D extent;
	};

	using RenderSystemList = std::vector<std::unique_ptr<RenderSystem>>;

	struct LightingData
	{
		FrameGraphResourceID lighting;
		VkImageView colorImageView;
		VkImageView depthImageView;
		VkExtent2D extent;
		RenderSystemList* renderSystems;
	};

	class LightingPass
	{
	public:
		LightingPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard, RenderSystemList& renderSystems)
		{
			//const auto& gBuffer = blackboard.get<GBufferData>();
			const auto& swapChain = blackboard.get<SwapChainData>();

			blackboard += frameGraph.addPass<LightingData>("Lighting",
				[&](FrameGraph::Builder& builder, LightingData& data)
				{
					//data.lighting = builder.create<FrameGraphTexture>("Lighting", { 1920, 1080 });
					//builder.declareWrite(data.lighting);

					//builder.declareRead(gBuffer.albedo);
					//builder.declareRead(gBuffer.normal);
					//builder.declareRead(gBuffer.depth);

					data.colorImageView = swapChain.colorImageView;
					data.depthImageView = swapChain.depthImageView;
					data.extent = swapChain.extent;
					data.renderSystems = &renderSystems;
				},
				[](const LightingData& data, const FrameInfo& frameInfo)
				{
					VkCommandBuffer commandBuffer = frameInfo.commandBuffer;

					VkRenderingAttachmentInfo colorAttachment{};
					colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
					colorAttachment.imageView = data.colorImageView;
					colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					colorAttachment.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };

					VkRenderingAttachmentInfo depthAttachment{};
					depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
					depthAttachment.imageView = data.depthImageView;
					depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					depthAttachment.clearValue.depthStencil = { 1.0f, 0 };

					VkRect2D renderArea{};
					renderArea.offset = { 0,0 };
					renderArea.extent = data.extent;

					VkRenderingInfo renderInfo{};
					renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
					renderInfo.renderArea = renderArea;
					renderInfo.layerCount = 1;
					renderInfo.colorAttachmentCount = 1;
					renderInfo.pColorAttachments = &colorAttachment;
					renderInfo.pDepthAttachment = &depthAttachment;

					vkCmdBeginRendering(commandBuffer, &renderInfo);

					VkViewport viewport{};
					viewport.x = 0.0f;
					viewport.y = 0.0f;
					viewport.width = static_cast<float>(data.extent.width);
					viewport.height = static_cast<float>(data.extent.height);
					viewport.minDepth = 0.0f;
					viewport.maxDepth = 1.0f;

					vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

					VkRect2D scissor{};
					scissor.offset = { 0, 0 };
					scissor.extent = data.extent;

					vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

					for (const auto& system : *data.renderSystems)
					{
						system->render(frameInfo);
					}

					vkCmdEndRendering(commandBuffer);
				});
		}
	};
}