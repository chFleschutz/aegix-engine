#include "pch.h"

#include "sky_box_pass.h"

#include "graphics/vulkan/vulkan_context.h"
#include "graphics/vulkan/vulkan_tools.h"

namespace Aegix::Graphics
{
	SkyBoxPass::SkyBoxPass()
	{
		m_descriptorSetLayout = DescriptorSetLayout::Builder{}
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.buildUnique();

		m_descriptorSet = std::make_unique<DescriptorSet>(*m_descriptorSetLayout);

		m_pipeline = Pipeline::GraphicsBuilder{}
			.addDescriptorSetLayout(*m_descriptorSetLayout)
			.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(SkyBoxUniforms))
			.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, SHADER_DIR "sky_box.vert.spv")
			.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "sky_box.frag.spv")
			.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
			.setDepthAttachment(VK_FORMAT_D32_SFLOAT)
			.setDepthTest(true, false, VK_COMPARE_OP_LESS_OR_EQUAL)
			.setCullMode(VK_CULL_MODE_FRONT_BIT)
			.setVertexBindingDescriptions({ { 0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX } })
			.setVertexAttributeDescriptions({ { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 } })
			.buildUnique();

		// Create vertex buffer
		{
			std::vector<glm::vec3> vertices{
				{-1.0f, -1.0f, -1.0f},
				{ 1.0f, -1.0f, -1.0f},
				{ 1.0f,  1.0f, -1.0f},
				{-1.0f,  1.0f, -1.0f},
				{-1.0f, -1.0f,  1.0f},
				{ 1.0f, -1.0f,  1.0f},
				{ 1.0f,  1.0f,  1.0f},
				{-1.0f,  1.0f,  1.0f}
			};

			VkDeviceSize vertexBufferSize = sizeof(glm::vec3) * vertices.size();
			m_vertexBuffer = std::make_unique<Buffer>(vertexBufferSize, 1,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
			m_vertexBuffer->upload(vertices.data(), vertexBufferSize);
		}

		// Create index buffer
		{
			std::vector<uint32_t> indices{
				0, 3, 2, 2, 1, 0,
				1, 2, 5, 5, 2, 6,
				4, 5, 6, 6, 7, 4,
				0, 4, 7, 7, 3, 0,
				2, 3, 7, 7, 6, 2,
				5, 4, 0, 0, 1, 5,
			};

			VkDeviceSize indexBufferSize = sizeof(uint32_t) * indices.size();
			m_indexBuffer = std::make_unique<Buffer>(indexBufferSize, 1,
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
			m_indexBuffer->upload(indices.data(), indexBufferSize);
		}
	}

	auto SkyBoxPass::createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo
	{
		m_sceneColor = builder.add(FrameGraphResourceCreateInfo{
			.name = "SceneColor",
			.type = FrameGraphResourceType::Reference,
			.usage = FrameGraphResourceUsage::ColorAttachment,
			});

		m_depth = builder.add(FrameGraphResourceCreateInfo{
			.name = "Depth",
			.type = FrameGraphResourceType::Reference,
			.usage = FrameGraphResourceUsage::DepthStencilAttachment,
			});

		return FrameGraphNodeCreateInfo{
			.name = "Sky Box",
			.inputs = { m_sceneColor, m_depth },
			.outputs = { m_sceneColor },
		};
	}

	void SkyBoxPass::execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo)
	{
		auto skyBoxEntity = frameInfo.scene.environment();
		if (!skyBoxEntity || !skyBoxEntity.has<Environment>())
			return;

		auto& environment = skyBoxEntity.get<Environment>();
		if (!environment.skybox)
			return;

		VkCommandBuffer cmd = frameInfo.cmd;

		DescriptorWriter{ *m_descriptorSetLayout }
			.writeImage(0, environment.skybox->descriptorImageInfo())
			.update(*m_descriptorSet);

		auto& sceneColorTexture = resources.texture(m_sceneColor);
		auto& depthTexture = resources.texture(m_depth);
		auto colorAttachment = Tools::renderingAttachmentInfo(sceneColorTexture, VK_ATTACHMENT_LOAD_OP_LOAD);
		auto depthAttachment = Tools::renderingAttachmentInfo(depthTexture, VK_ATTACHMENT_LOAD_OP_LOAD);

		VkRenderingInfo renderInfo{};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderInfo.renderArea.offset = { 0, 0 };
		renderInfo.renderArea.extent = frameInfo.swapChainExtent;
		renderInfo.layerCount = 1;
		renderInfo.pColorAttachments = &colorAttachment;
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pDepthAttachment = &depthAttachment;

		vkCmdBeginRendering(cmd, &renderInfo);
		Tools::vk::cmdViewport(cmd, frameInfo.swapChainExtent);
		Tools::vk::cmdScissor(cmd, frameInfo.swapChainExtent);

		auto& camera = frameInfo.scene.mainCamera().get<Camera>();
		SkyBoxUniforms uniforms{
			.view = camera.viewMatrix,
			.projection = camera.projectionMatrix
		};
		
		m_pipeline->bind(cmd);
		m_pipeline->pushConstants(cmd, VK_SHADER_STAGE_VERTEX_BIT, uniforms);
		m_pipeline->bindDescriptorSet(cmd, 0, *m_descriptorSet);

		VkBuffer vertexBuffers[] = { *m_vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(cmd, *m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmd, 36, 1, 0, 0, 0);

		vkCmdEndRendering(cmd);
	}
}
