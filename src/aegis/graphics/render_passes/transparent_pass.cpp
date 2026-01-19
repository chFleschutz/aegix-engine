#include "pch.h"
#include "transparent_pass.h"

#include "graphics/vulkan/vulkan_tools.h"
#include "scene/components.h"

namespace Aegis::Graphics
{
	TransparentPass::TransparentPass(FGResourcePool& pool) :
		m_globalUbo{ Buffer::uniformBuffer(sizeof(TransparentUbo)) },
		m_globalSetLayout{ createDescriptorSetLayout() }
	{
		m_globalSets.reserve(MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_globalSets.emplace_back(m_globalSetLayout);
			DescriptorWriter{ m_globalSetLayout }
				.writeBuffer(0, m_globalUbo, i)
				.update(m_globalSets[i]);
		}

		m_sceneColor = pool.addReference("SceneColor",
			FGResource::Usage::ColorAttachment);

		m_depth = pool.addReference("Depth",
			FGResource::Usage::DepthStencilAttachment);
	}

	auto TransparentPass::info() -> FGNode::Info
	{
		return FGNode::Info{
			.name = "Transparent",
			.reads = { m_depth },
			.writes = { m_sceneColor }
		};
	}

	void TransparentPass::execute(FGResourcePool& pool, const FrameInfo& frameInfo)
	{
		updateUBO(frameInfo);

		auto& sceneColor = pool.texture(m_sceneColor);
		auto& depth = pool.texture(m_depth);
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

		VkCommandBuffer cmd = frameInfo.cmd;
		vkCmdBeginRendering(cmd, &renderingInfo);
		{
			Tools::vk::cmdViewport(cmd, extent);
			Tools::vk::cmdScissor(cmd, extent);

			RenderContext ctx{
				.scene = frameInfo.scene,
				.ui = frameInfo.ui,
				.frameIndex = frameInfo.frameIndex,
				.cmd = cmd,
				.globalSet = m_globalSets[frameInfo.frameIndex]
			};

			for (const auto& system : m_renderSystems)
			{
				system->render(ctx);
			}
		}
		vkCmdEndRendering(cmd);
	}

	auto TransparentPass::createDescriptorSetLayout() -> DescriptorSetLayout
	{
		return DescriptorSetLayout::Builder{}
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
			.build();
	}

	void TransparentPass::updateUBO(const FrameInfo& frameInfo)
	{
		Scene::Entity mainCamera = frameInfo.scene.mainCamera();
		if (!mainCamera)
			return;

		// TODO: Check if these need to be transposed for shaders (row-major layout)
		auto& camera = mainCamera.get<Camera>();
		TransparentUbo ubo{
			.view = camera.viewMatrix,
			.projection = camera.projectionMatrix
		};

		m_globalUbo.writeToIndex(&ubo, frameInfo.frameIndex);
	}
}
