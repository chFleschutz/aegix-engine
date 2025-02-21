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
			FrameGraphResourceID emissive, FrameGraphResourceID depth)
		{
			auto& renderer = blackboard.get<RendererData>();

			blackboard += frameGraph.addPass<GBufferData>("GBuffer",
				[&](FrameGraph::Builder& builder, GBufferData& data)
				{
					auto& stage = frameGraph.resourcePool().renderStage(RenderStage::Type::Geometry);

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
					VkCommandBuffer commandBuffer = frameInfo.commandBuffer;
					auto& stage = resources.renderStage(RenderStage::Type::Geometry);

					updateUBO(stage, frameInfo);

					auto& position = resources.texture(data.position);
					auto& normal = resources.texture(data.normal);
					auto& albedo = resources.texture(data.albedo);
					auto& arm = resources.texture(data.arm);
					auto& emissive = resources.texture(data.emissive);
					auto& depth = resources.texture(data.depth);

					VkExtent2D extent = albedo.texture.extent();

					auto colorAttachments = std::array{
						Tools::renderingAttachmentInfo(position.texture, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 0.0f, 0.0f }),
						Tools::renderingAttachmentInfo(normal.texture, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 0.0f, 0.0f }),
						Tools::renderingAttachmentInfo(albedo.texture, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 0.0f, 0.0f }),
						Tools::renderingAttachmentInfo(arm.texture, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 0.0f, 0.0f }),
						Tools::renderingAttachmentInfo(emissive.texture, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 0.0f, 0.0f })
					};

					VkRenderingAttachmentInfo depthAttachment = Tools::renderingAttachmentInfo(
						depth.texture, VK_ATTACHMENT_LOAD_OP_CLEAR, { 1.0f, 0 });

					VkRenderingInfo renderInfo{};
					renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
					renderInfo.renderArea.offset = { 0, 0 };
					renderInfo.renderArea.extent = extent;
					renderInfo.layerCount = 1;
					renderInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
					renderInfo.pColorAttachments = colorAttachments.data();
					renderInfo.pDepthAttachment = &depthAttachment;

					vkCmdBeginRendering(commandBuffer, &renderInfo);

					Tools::vk::cmdViewport(commandBuffer, extent);
					Tools::vk::cmdScissor(commandBuffer, extent);

					VkDescriptorSet globalSet = stage.descriptorSet->descriptorSet(frameInfo.frameIndex);
					for (const auto& system : stage.renderSystems)
					{
						system->render(frameInfo, globalSet);
					}

					vkCmdEndRendering(commandBuffer);
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