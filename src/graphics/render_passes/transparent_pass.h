#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	struct TransparentUbo
	{
		glm::mat4 view{ 1.0f };
		glm::mat4 projection{ 1.0f };
	};

	class TransparentPass : public FrameGraphRenderPass
	{
	public:
		TransparentPass(FrameGraph& framegraph, VulkanDevice& device, DescriptorPool& pool)
		{
			auto& stage = framegraph.resourcePool().renderStage(RenderStage::Type::Transparency);

			stage.descriptorSetLayout = DescriptorSetLayout::Builder(device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.build();

			stage.ubo = std::make_unique<UniformBuffer>(device, TransparentUbo{});

			stage.descriptorSet = DescriptorSet::Builder(device, pool, *stage.descriptorSetLayout)
				.addBuffer(0, *stage.ubo)
				.build();
		}

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override
		{
			m_sceneColor = builder.add({ "SceneColor",
				FrameGraphResourceType::Reference,
				FrameGraphResourceUsage::ColorAttachment
				});
			m_depth = builder.add({ "Depth",
				FrameGraphResourceType::Reference,
				FrameGraphResourceUsage::DepthStencilAttachment
				});

			return FrameGraphNodeCreateInfo{
				.name = "Transparent Pass",
				.inputs = { m_sceneColor, m_depth },
				.outputs = { m_sceneColor }
			};
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
			VkCommandBuffer commandBuffer = frameInfo.commandBuffer;

			auto& stage = resources.renderStage(RenderStage::Type::Transparency);
			updateUBO(stage, frameInfo);

			auto& sceneColor = resources.texture(m_sceneColor);
			auto& depth = resources.texture(m_depth);
			auto colorAttachment = Tools::renderingAttachmentInfo(sceneColor, VK_ATTACHMENT_LOAD_OP_LOAD, {});
			auto depthAttachment = Tools::renderingAttachmentInfo(depth, VK_ATTACHMENT_LOAD_OP_LOAD, {});

			VkExtent2D extent = frameInfo.swapChainExtent;
			VkRenderingInfo renderingInfo{};
			renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
			renderingInfo.renderArea = { 0, 0, extent.width, extent.height };
			renderingInfo.layerCount = 1;
			renderingInfo.colorAttachmentCount = 1;
			renderingInfo.pColorAttachments = &colorAttachment;
			renderingInfo.pDepthAttachment = &depthAttachment;

			vkCmdBeginRendering(commandBuffer, &renderingInfo);

			Tools::vk::cmdViewport(commandBuffer, extent);
			Tools::vk::cmdScissor(commandBuffer, extent);

			for (const auto& system : stage.renderSystems)
			{
				system->render(frameInfo, stage.descriptorSet->descriptorSet(frameInfo.frameIndex));
			}

			vkCmdEndRendering(commandBuffer);
		}

	private:
		void updateUBO(RenderStage& stage, const FrameInfo& frameInfo)
		{
			auto& camera = frameInfo.scene.camera().getComponent<Component::Camera>();

			TransparentUbo ubo{
				.view = camera.viewMatrix,
				.projection = camera.projectionMatrix
			};

			stage.ubo->setData(ubo);
		}

		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_depth;
	};
}