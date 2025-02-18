#pragma once

#include "graphics/frame_graph/frame_graph.h"
#include "graphics/frame_graph/frame_graph_blackboard.h"
#include "graphics/systems/render_system.h"

#include <array>
#include <cassert>

namespace Aegix::Graphics
{
	using RenderSystemList = std::vector<std::unique_ptr<RenderSystem>>;

	struct GBufferData
	{
		FrameGraphResourceID position;
		FrameGraphResourceID normal;
		FrameGraphResourceID albedo;
		FrameGraphResourceID arm;
		FrameGraphResourceID emissive;
		FrameGraphResourceID depth;
		RenderSystemList* renderSystems;
	};

	class GBufferPass
	{
	public:
		GBufferPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard, RenderSystemList& renderSystems,
			FrameGraphResourceID position, FrameGraphResourceID normal, FrameGraphResourceID albedo, FrameGraphResourceID arm, 
			FrameGraphResourceID emissive, FrameGraphResourceID depth )
		{
			blackboard += frameGraph.addPass<GBufferData>("GBuffer",
				[&](FrameGraph::Builder& builder, GBufferData& data)
				{
					data.position = builder.declareWrite(position);
					data.normal = builder.declareWrite(normal);
					data.albedo = builder.declareWrite(albedo);
					data.arm = builder.declareWrite(arm);
					data.emissive = builder.declareWrite(emissive);
					data.depth = builder.declareWrite(depth);

					data.renderSystems = &renderSystems;
				},
				[](const GBufferData& data, const FrameGraphResourcePool& resources, const FrameInfo& frameInfo)
				{
					const auto& position = resources.getTexture(data.position);
					const auto& normal = resources.getTexture(data.normal);
					const auto& albedo = resources.getTexture(data.albedo);
					const auto& arm = resources.getTexture(data.arm);
					const auto& emissive = resources.getTexture(data.emissive);
					const auto& depth = resources.getTexture(data.depth);

					VkExtent2D extent = albedo.texture.extent();
					assert(extent.width == position.texture.extent().width && extent.height == position.texture.extent().height);
					assert(extent.width == normal.texture.extent().width && extent.height == normal.texture.extent().height);
					assert(extent.width == arm.texture.extent().width && extent.height == arm.texture.extent().height);
					assert(extent.width == emissive.texture.extent().width && extent.height == emissive.texture.extent().height);
					assert(extent.width == depth.texture.extent().width && extent.height == depth.texture.extent().height);

					VkCommandBuffer commandBuffer = frameInfo.commandBuffer;

					// TODO: Automate layout transitions (and remove the const_casts)
					const_cast<Texture&>(position.texture).transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
					const_cast<Texture&>(normal.texture).transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
					const_cast<Texture&>(albedo.texture).transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
					const_cast<Texture&>(arm.texture).transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
					const_cast<Texture&>(emissive.texture).transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

					std::array<VkRenderingAttachmentInfo, 5> colorAttachments{};
					colorAttachments[0].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
					colorAttachments[0].imageView = position.texture.imageView();
					colorAttachments[0].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					colorAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					colorAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					colorAttachments[0].clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };

					colorAttachments[1].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
					colorAttachments[1].imageView = normal.texture.imageView();
					colorAttachments[1].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					colorAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					colorAttachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					colorAttachments[1].clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };

					colorAttachments[2].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
					colorAttachments[2].imageView = albedo.texture.imageView();
					colorAttachments[2].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					colorAttachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					colorAttachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					colorAttachments[2].clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };

					colorAttachments[3].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
					colorAttachments[3].imageView = arm.texture.imageView();
					colorAttachments[3].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					colorAttachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					colorAttachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					colorAttachments[3].clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };

					colorAttachments[4].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
					colorAttachments[4].imageView = emissive.texture.imageView();
					colorAttachments[4].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					colorAttachments[4].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					colorAttachments[4].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					colorAttachments[4].clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };

					VkRenderingAttachmentInfo depthAttachment{};
					depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
					depthAttachment.imageView = depth.texture.imageView();
					depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					depthAttachment.clearValue.depthStencil = { 1.0f, 0 };

					VkRect2D renderArea{};
					renderArea.offset = { 0,0 };
					renderArea.extent = extent;

					VkRenderingInfo renderInfo{};
					renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
					renderInfo.renderArea = renderArea;
					renderInfo.layerCount = 1;
					renderInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
					renderInfo.pColorAttachments = colorAttachments.data();
					renderInfo.pDepthAttachment = &depthAttachment;

					vkCmdBeginRendering(commandBuffer, &renderInfo);

					VkViewport viewport{};
					viewport.x = 0.0f;
					viewport.y = 0.0f;
					viewport.width = static_cast<float>(extent.width);
					viewport.height = static_cast<float>(extent.height);
					viewport.minDepth = 0.0f;
					viewport.maxDepth = 1.0f;

					vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

					VkRect2D scissor{};
					scissor.offset = { 0, 0 };
					scissor.extent = extent;

					vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

					for (const auto& system : *data.renderSystems)
					{
						system->render(frameInfo);
					}

					vkCmdEndRendering(commandBuffer);

					// TODO: Automate layout transitions (and remove the const_casts)
					const_cast<Texture&>(albedo.texture).transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					const_cast<Texture&>(normal.texture).transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				});
		}
	};
}