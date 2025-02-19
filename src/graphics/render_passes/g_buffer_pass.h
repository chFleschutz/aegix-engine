#pragma once

#include "graphics/frame_graph/frame_graph.h"
#include "graphics/frame_graph/frame_graph_blackboard.h"
#include "graphics/systems/render_system.h"

#include <array>
#include <cassert>

namespace Aegix::Graphics
{
	struct GBufferData
	{
		FrameGraphResourceID position;
		FrameGraphResourceID normal;
		FrameGraphResourceID albedo;
		FrameGraphResourceID arm;
		FrameGraphResourceID emissive;
		FrameGraphResourceID depth;
	};

	class GBufferPass
	{
	public:
		GBufferPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard,
			FrameGraphResourceID position, FrameGraphResourceID normal, FrameGraphResourceID albedo, FrameGraphResourceID arm, 
			FrameGraphResourceID emissive, FrameGraphResourceID depth )
		{
			auto& renderer = blackboard.get<RendererData>();

			blackboard += frameGraph.addPass<GBufferData>("GBuffer",
				[&](FrameGraph::Builder& builder, GBufferData& data)
				{
					auto& stage = frameGraph.resourcePool().renderStage(RenderStageType::Geometry);

					stage.descriptorSetLayout = DescriptorSetLayout::Builder(renderer.device)
						.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
						.build();

					stage.ubo = std::make_unique<UniformBuffer>(renderer.device, GlobalUbo{});

					stage.descriptorSet = DescriptorSet::Builder(renderer.device, renderer.pool, *stage.descriptorSetLayout)
						.addBuffer(0, *stage.ubo)
						.build();

					data.position = builder.declareWrite(position);
					data.normal = builder.declareWrite(normal);
					data.albedo = builder.declareWrite(albedo);
					data.arm = builder.declareWrite(arm);
					data.emissive = builder.declareWrite(emissive);
					data.depth = builder.declareWrite(depth);
				},
				[](const GBufferData& data, FrameGraphResourcePool& resources, const FrameInfo& frameInfo)
				{
					auto& stage = resources.renderStage(RenderStageType::Geometry);
					updateUBO(stage, frameInfo);

					const auto& position = resources.texture(data.position);
					const auto& normal = resources.texture(data.normal);
					const auto& albedo = resources.texture(data.albedo);
					const auto& arm = resources.texture(data.arm);
					const auto& emissive = resources.texture(data.emissive);
					const auto& depth = resources.texture(data.depth);

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

					VkDescriptorSet globalSet = stage.descriptorSet->descriptorSet(frameInfo.frameIndex);
					for (const auto& system : stage.renderSystems)
					{
						system->render(frameInfo, globalSet);
					}

					vkCmdEndRendering(commandBuffer);

					// TODO: Automate layout transitions (and remove the const_casts)
					const_cast<Texture&>(position.texture).transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					const_cast<Texture&>(normal.texture).transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					const_cast<Texture&>(albedo.texture).transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					const_cast<Texture&>(arm.texture).transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					const_cast<Texture&>(emissive.texture).transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				});
		}

	private:
		static void updateUBO(RenderStage& stage, const FrameInfo& frameInfo)
		{
			auto& camera = frameInfo.scene.camera().getComponent<Component::Camera>();
			camera.aspect = frameInfo.aspectRatio;

			GlobalUbo ubo{
				.projection = camera.projectionMatrix,
				.view = camera.viewMatrix,
				.inverseView = camera.inverseViewMatrix
			};

			stage.ubo->setData(frameInfo.frameIndex, ubo);
		}
	};
}