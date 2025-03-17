#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/systems/render_system.h"
#include "graphics/vulkan_tools.h"

namespace Aegix::Graphics
{
	struct GBufferUbo
	{
		glm::mat4 projection{ 1.0f };
		glm::mat4 view{ 1.0f };
		glm::mat4 inverseView{ 1.0f };
	};

	class GeometryPass : public FrameGraphRenderPass
	{
	public:
		GeometryPass(FrameGraph& framegraph, VulkanDevice& device, DescriptorPool& pool)
		{
			auto& stage = framegraph.resourcePool().renderStage(RenderStage::Type::Geometry);

			stage.descriptorSetLayout = DescriptorSetLayout::Builder(device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.build();

			auto aligment = device.properties().limits.minUniformBufferOffsetAlignment;
			stage.ubo = std::make_unique<Buffer>(device, sizeof(GBufferUbo), MAX_FRAMES_IN_FLIGHT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, aligment);

			stage.descriptorSet = DescriptorSet::Builder(device, pool, *stage.descriptorSetLayout)
				.addBuffer(0, *stage.ubo)
				.build();
		}

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override
		{
			m_position = builder.add(FrameGraphResourceCreateInfo{
				.name = "Position",
				.type = FrameGraphResourceType::Texture,
				.usage = FrameGraphResourceUsage::ColorAttachment,
				.info = FrameGraphResourceTextureInfo{
					.format = VK_FORMAT_R16G16B16A16_SFLOAT,
					.extent = { 0, 0 },
					.resizePolicy = ResizePolicy::SwapchainRelative
					}
				});

			m_normal = builder.add(FrameGraphResourceCreateInfo{
				.name = "Normal",
				.type = FrameGraphResourceType::Texture,
				.usage = FrameGraphResourceUsage::ColorAttachment,
				.info = FrameGraphResourceTextureInfo{
					.format = VK_FORMAT_R16G16B16A16_SFLOAT,
					.extent = { 0, 0 },
					.resizePolicy = ResizePolicy::SwapchainRelative
					}
				});

			m_albedo = builder.add(FrameGraphResourceCreateInfo{
				.name = "Albedo",
				.type = FrameGraphResourceType::Texture,
				.usage = FrameGraphResourceUsage::ColorAttachment,
				.info = FrameGraphResourceTextureInfo{
					.format = VK_FORMAT_R8G8B8A8_UNORM,
					.extent = { 0, 0 },
					.resizePolicy = ResizePolicy::SwapchainRelative
					}
				});

			m_arm = builder.add(FrameGraphResourceCreateInfo{
				.name = "ARM",
				.type = FrameGraphResourceType::Texture,
				.usage = FrameGraphResourceUsage::ColorAttachment,
				.info = FrameGraphResourceTextureInfo{
					.format = VK_FORMAT_R8G8B8A8_UNORM,
					.extent = { 0, 0 },
					.resizePolicy = ResizePolicy::SwapchainRelative
					}
				});

			m_emissive = builder.add(FrameGraphResourceCreateInfo{
				.name = "Emissive",
				.type = FrameGraphResourceType::Texture,
				.usage = FrameGraphResourceUsage::ColorAttachment,
				.info = FrameGraphResourceTextureInfo{
					.format = VK_FORMAT_R8G8B8A8_UNORM,
					.extent = { 0, 0 },
					.resizePolicy = ResizePolicy::SwapchainRelative
					}
				});

			m_depth = builder.add(FrameGraphResourceCreateInfo{
				.name = "Depth",
				.type = FrameGraphResourceType::Texture,
				.usage = FrameGraphResourceUsage::DepthStencilAttachment,
				.info = FrameGraphResourceTextureInfo{
					.format = VK_FORMAT_D32_SFLOAT,
					.extent = { 0, 0 },
					.resizePolicy = ResizePolicy::SwapchainRelative,
					}
				});

			FrameGraphNodeCreateInfo info{};
			info.name = "Geometry";
			info.inputs = {};
			info.outputs = { m_position, m_normal, m_albedo, m_arm, m_emissive, m_depth };
			return info;
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
			VkCommandBuffer commandBuffer = frameInfo.commandBuffer;
			auto& stage = resources.renderStage(RenderStage::Type::Geometry);
			updateUBO(stage, frameInfo);

			auto& position = resources.texture(m_position);
			auto& normal = resources.texture(m_normal);
			auto& albedo = resources.texture(m_albedo);
			auto& arm = resources.texture(m_arm);
			auto& emissive = resources.texture(m_emissive);
			auto& depth = resources.texture(m_depth);

			VkExtent2D extent = albedo.extent2D();

			auto colorAttachments = std::array{
				Tools::renderingAttachmentInfo(position, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 0.0f, 0.0f }),
				Tools::renderingAttachmentInfo(normal, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 0.0f, 0.0f }),
				Tools::renderingAttachmentInfo(albedo, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 0.0f, 0.0f }),
				Tools::renderingAttachmentInfo(arm, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 0.0f, 0.0f }),
				Tools::renderingAttachmentInfo(emissive, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 0.0f, 0.0f })
			};

			VkRenderingAttachmentInfo depthAttachment = Tools::renderingAttachmentInfo(
				depth, VK_ATTACHMENT_LOAD_OP_CLEAR, { 1.0f, 0 });

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
		}

	private:
		void updateUBO(RenderStage& stage, const FrameInfo& frameInfo)
		{
			Scene::Entity mainCamera = frameInfo.scene.mainCamera();
			if (!mainCamera)
				return;

			auto& camera = mainCamera.component<Camera>();
			camera.aspect = frameInfo.aspectRatio;

			GBufferUbo ubo{
				.projection = camera.projectionMatrix,
				.view = camera.viewMatrix,
				.inverseView = camera.inverseViewMatrix
			};

			stage.ubo->writeToIndex(&ubo, frameInfo.frameIndex);
		}

		FrameGraphResourceHandle m_position;
		FrameGraphResourceHandle m_normal;
		FrameGraphResourceHandle m_albedo;
		FrameGraphResourceHandle m_arm;
		FrameGraphResourceHandle m_emissive;
		FrameGraphResourceHandle m_depth;
	};
}