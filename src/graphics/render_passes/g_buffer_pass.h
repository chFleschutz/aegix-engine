#pragma once

#include "graphics/frame_graph/frame_graph.h"
#include "graphics/frame_graph/frame_graph_blackboard.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/systems/render_system.h"
#include "graphics/vulkan_tools.h"

#include <array>

namespace Aegix::Graphics
{
	class GBufferPass : public FrameGraphRenderPass
	{
	public:
		GBufferPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
		{
			auto& renderer = blackboard.get<RendererData>();
			auto& stage = frameGraph.resourcePool().renderStage(RenderStage::Type::Geometry);

			stage.descriptorSetLayout = DescriptorSetLayout::Builder(renderer.device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.build();

			stage.ubo = std::make_unique<UniformBuffer>(renderer.device, GlobalUbo{});

			stage.descriptorSet = DescriptorSet::Builder(renderer.device, renderer.pool, *stage.descriptorSetLayout)
				.addBuffer(0, *stage.ubo)
				.build();
		}

		virtual auto createInfo(FrameGraphResourcePool& pool) -> FrameGraphNodeCreateInfo override
		{
			m_position = pool.addResource(FrameGraphResourceCreateInfo{
				.name = "Position",
				.type = FrameGraphResourceType::Texture,
				.info = FrameGraphResourceTextureInfo{
					.extent = { 0, 0 },
					.format = VK_FORMAT_R16G16B16A16_SFLOAT,
					.resizePolicy = ResizePolicy::SwapchainRelative
					}
				});

			m_normal = pool.addResource(FrameGraphResourceCreateInfo{
				.name = "Normal",
				.type = FrameGraphResourceType::Texture,
				.info = FrameGraphResourceTextureInfo{
					.extent = { 0, 0 },
					.format = VK_FORMAT_R16G16B16A16_SFLOAT,
					.resizePolicy = ResizePolicy::SwapchainRelative
					}
				});

			m_albedo = pool.addResource(FrameGraphResourceCreateInfo{
				.name = "Albedo",
				.type = FrameGraphResourceType::Texture,
				.info = FrameGraphResourceTextureInfo{
					.extent = { 0, 0 },
					.format = VK_FORMAT_R8G8B8A8_UNORM,
					.resizePolicy = ResizePolicy::SwapchainRelative
					}
				});

			m_arm = pool.addResource(FrameGraphResourceCreateInfo{
				.name = "ARM",
				.type = FrameGraphResourceType::Texture,
				.info = FrameGraphResourceTextureInfo{
					.extent = { 0, 0 },
					.format = VK_FORMAT_R8G8B8A8_UNORM,
					.resizePolicy = ResizePolicy::SwapchainRelative
					}
				});

			m_emissive = pool.addResource(FrameGraphResourceCreateInfo{
				.name = "Emissive",
				.type = FrameGraphResourceType::Texture,
				.info = FrameGraphResourceTextureInfo{
					.extent = { 0, 0 },
					.format = VK_FORMAT_R8G8B8A8_UNORM,
					.resizePolicy = ResizePolicy::SwapchainRelative
					}
				});

			m_depth = pool.addResource(FrameGraphResourceCreateInfo{
				.name = "Depth",
				.type = FrameGraphResourceType::Texture,
				.info = FrameGraphResourceTextureInfo{
					.extent = { 0, 0 },
					.format = VK_FORMAT_D32_SFLOAT,
					.resizePolicy = ResizePolicy::SwapchainRelative,
					.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
					}
				});

			FrameGraphNodeCreateInfo info{};
			info.name = "GBuffer Pass";
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

			VkExtent2D extent = albedo.extent();

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
			auto& camera = frameInfo.scene.camera().getComponent<Component::Camera>();
			camera.aspect = frameInfo.aspectRatio;

			GlobalUbo ubo{
				.projection = camera.projectionMatrix,
				.view = camera.viewMatrix,
				.inverseView = camera.inverseViewMatrix
			};

			stage.ubo->setData(frameInfo.frameIndex, ubo);
		}

		FrameGraphResourceHandle m_position;
		FrameGraphResourceHandle m_normal;
		FrameGraphResourceHandle m_albedo;
		FrameGraphResourceHandle m_arm;
		FrameGraphResourceHandle m_emissive;
		FrameGraphResourceHandle m_depth;
	};
}