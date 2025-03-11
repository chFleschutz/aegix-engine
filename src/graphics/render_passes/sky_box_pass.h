#pragma once

#include "graphics/descriptors.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/pipeline.h"

namespace Aegix::Graphics
{
	struct SkyBoxUniforms
	{
		glm::mat4 view;
		glm::mat4 projection;
	};

	class SkyBoxPass : public FrameGraphRenderPass
	{
	public:
		SkyBoxPass(VulkanDevice& device, DescriptorPool& pool)
			: m_skyBoxTexture{ device }
		{
			// TODO: Remove hard-coded path
			//m_skyBoxTexture.createCube("C:/Users/chfle/Downloads/autumn_field_puresky_2k.hdr");

			m_descriptorSetLayout = DescriptorSetLayout::Builder(device)
				//.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.build();

			m_descriptorSet = DescriptorSet::Builder(device, pool, *m_descriptorSetLayout)
				//.addTexture(0, m_skyBoxTexture)
				.build();

			m_pipelineLayout = PipelineLayout::Builder(device)
				.addDescriptorSetLayout(*m_descriptorSetLayout)
				.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(SkyBoxUniforms))
				.build();

			m_pipeline = Pipeline::GraphicsBuilder(device, *m_pipelineLayout)
				.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, SHADER_DIR "sky_box.vert.spv")
				.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "sky_box.frag.spv")
				.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
				.setDepthAttachment(VK_FORMAT_D32_SFLOAT)
				.setDepthTest(true, false, VK_COMPARE_OP_LESS_OR_EQUAL)
				.setCullMode(VK_CULL_MODE_FRONT_BIT)
				.setVertexBindingDescriptions({ { 0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX } })
				.setVertexAttributeDescriptions({ { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 } })
				.build();

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

				auto stagingBuffer = Buffer(device, sizeof(glm::vec3), (uint32_t)vertices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
				stagingBuffer.map();
				stagingBuffer.writeToBuffer(vertices.data());

				m_vertexBuffer = std::make_unique<Buffer>(device, sizeof(glm::vec3), (uint32_t)vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
				device.copyBuffer(stagingBuffer.buffer(), m_vertexBuffer->buffer(), vertices.size() * sizeof(glm::vec3));
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

				auto stagingBuffer = Buffer(device, sizeof(uint32_t), (uint32_t)indices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
				stagingBuffer.map();
				stagingBuffer.writeToBuffer(indices.data());

				m_indexBuffer = std::make_unique<Buffer>(device, sizeof(uint32_t), (uint32_t)indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
				device.copyBuffer(stagingBuffer.buffer(), m_indexBuffer->buffer(), indices.size() * sizeof(uint32_t));
			}
		}

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo
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

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
			VkCommandBuffer cmd = frameInfo.commandBuffer;

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

			m_pipeline->bind(cmd);

			auto& camera = frameInfo.scene.mainCamera().component<Camera>();
			SkyBoxUniforms uniforms{
				.view = camera.viewMatrix,
				.projection = camera.projectionMatrix
			};
			Tools::vk::cmdPushConstants(cmd, *m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, uniforms);

			Tools::vk::cmdBindDescriptorSet(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pipelineLayout,
				m_descriptorSet->descriptorSet(frameInfo.frameIndex));

			VkBuffer vertexBuffers[] = { m_vertexBuffer->buffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(cmd, m_indexBuffer->buffer(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(cmd, 36, 1, 0, 0, 0);

			vkCmdEndRendering(cmd);
		}

	private:
		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_depth;

		std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
		std::unique_ptr<DescriptorSet> m_descriptorSet;
		std::unique_ptr<PipelineLayout> m_pipelineLayout;
		std::unique_ptr<Pipeline> m_pipeline;

		std::unique_ptr<Buffer> m_vertexBuffer;
		std::unique_ptr<Buffer> m_indexBuffer;

		Texture m_skyBoxTexture;
	};
}
