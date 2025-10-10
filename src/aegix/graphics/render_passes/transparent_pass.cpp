#include "transparent_pass.h"

#include "graphics/vulkan_tools.h"
#include "scene/components.h"

namespace Aegix::Graphics
{
	TransparentPass::TransparentPass(FrameGraph& framegraph)
		: m_globalUbo{ Buffer::createUniformBuffer(sizeof(TransparentUbo)) }
	{
		m_globalSetLayout = DescriptorSetLayout::Builder{}
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.buildUnique();

		m_globalSet = DescriptorSet::Builder{ *m_globalSetLayout }
			.addBuffer(0, m_globalUbo)
			.buildUnique();


		//auto& stage = framegraph.resourcePool().renderStage(RenderStage::Type::Transparency);

		//stage.descriptorSetLayout = DescriptorSetLayout::Builder{}
		//	.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
		//	.buildUnique();

		//auto aligment = VulkanContext::device().properties().limits.minUniformBufferOffsetAlignment;
		//stage.ubo = std::make_unique<Buffer>(sizeof(GBufferUbo), MAX_FRAMES_IN_FLIGHT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		//	VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, aligment);

		//stage.descriptorSet = DescriptorSet::Builder(VulkanContext::descriptorPool(), *stage.descriptorSetLayout)
		//	.addBuffer(0, *stage.ubo)
		//	.buildUnique();
	}

	auto TransparentPass::createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo
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
			.name = "Transparent",
			.inputs = { m_sceneColor, m_depth },
			.outputs = { m_sceneColor }
		};
	}

	void TransparentPass::execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo)
	{
		VkCommandBuffer cmd = frameInfo.cmd;

		auto& stage = resources.renderStage(RenderStage::Type::Transparency);
		updateUBO(frameInfo);

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

		vkCmdBeginRendering(cmd, &renderingInfo);
		{
			Tools::vk::cmdViewport(cmd, extent);
			Tools::vk::cmdScissor(cmd, extent);

			RenderContext ctx{
				.scene = frameInfo.scene,
				.ui = frameInfo.ui,
				.frameIndex = frameInfo.frameIndex,
				.cmd = cmd,
				.globalSet = m_globalSet->descriptorSet(frameInfo.frameIndex)
			};

			for (const auto& system : m_renderSystems)
			{
				system->render(ctx);
			}
		}
		vkCmdEndRendering(cmd);
	}

	void TransparentPass::updateUBO(const FrameInfo& frameInfo)
	{
		Scene::Entity mainCamera = frameInfo.scene.mainCamera();
		if (!mainCamera)
			return;

		auto& camera = mainCamera.get<Camera>();
		TransparentUbo ubo{
			.view = camera.viewMatrix,
			.projection = camera.projectionMatrix
		};

		m_globalUbo.writeToIndex(&ubo, frameInfo.frameIndex);
	}
}
