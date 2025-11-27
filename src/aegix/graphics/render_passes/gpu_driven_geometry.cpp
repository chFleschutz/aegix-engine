#include "pch.h"
#include "gpu_driven_geometry.h"

#include "graphics/vulkan/vulkan_tools.h"

namespace Aegix::Graphics
{
	GPUDrivenGeometry::GPUDrivenGeometry(FGResourcePool& pool)
	{
		m_position = pool.addImage("Position",
			FGResource::Usage::ColorAttachment,
			FGTextureInfo{
				.format = VK_FORMAT_R16G16B16A16_SFLOAT,
				.resizeMode = FGResizeMode::SwapChainRelative
			});

		m_normal = pool.addImage("Normal",
			FGResource::Usage::ColorAttachment,
			FGTextureInfo{
				.format = VK_FORMAT_R16G16B16A16_SFLOAT,
				.resizeMode = FGResizeMode::SwapChainRelative
			});

		m_albedo = pool.addImage("Albedo",
			FGResource::Usage::ColorAttachment,
			FGTextureInfo{
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.resizeMode = FGResizeMode::SwapChainRelative
			});

		m_arm = pool.addImage("ARM",
			FGResource::Usage::ColorAttachment,
			FGTextureInfo{
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.resizeMode = FGResizeMode::SwapChainRelative
			});

		m_emissive = pool.addImage("Emissive",
			FGResource::Usage::ColorAttachment,
			FGTextureInfo{
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.resizeMode = FGResizeMode::SwapChainRelative
			});

		m_depth = pool.addImage("Depth",
			FGResource::Usage::DepthStencilAttachment,
			FGTextureInfo{
				.format = VK_FORMAT_D32_SFLOAT,
				.resizeMode = FGResizeMode::SwapChainRelative
			});

		m_visibleInstances = pool.addReference("VisibleInstances",
			FGResource::Usage::ComputeReadStorage);

		m_instanceData = pool.addReference("InstanceData",
			FGResource::Usage::ComputeReadStorage);
	}

	auto GPUDrivenGeometry::info() -> FGNode::Info 
	{
		return FGNode::Info{
			.name = "GPU Driven Geometry Pass",
			.reads = { m_instanceData, m_visibleInstances },
			.writes = { m_position, m_normal, m_albedo, m_arm, m_emissive, m_depth }
		};
	}

	void GPUDrivenGeometry::execute(FGResourcePool& pool, const FrameInfo& frameInfo)
	{
		const auto& position = pool.texture(m_position);
		const auto& normal = pool.texture(m_normal);
		const auto& albedo = pool.texture(m_albedo);
		const auto& arm = pool.texture(m_arm);
		const auto& emissive = pool.texture(m_emissive);
		const auto& depth = pool.texture(m_depth);

		VkRect2D renderArea{
			.offset = { 0, 0 },
			.extent = albedo.extent2D()
		};

		auto colorAttachments = std::array{
			Tools::renderingAttachmentInfo(position, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 0.0f, 0.0f }),
			Tools::renderingAttachmentInfo(normal, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 0.0f, 0.0f }),
			Tools::renderingAttachmentInfo(albedo, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 0.0f, 0.0f }),
			Tools::renderingAttachmentInfo(arm, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 0.0f, 0.0f }),
			Tools::renderingAttachmentInfo(emissive, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 0.0f, 0.0f })
		};
		auto depthAttachment = Tools::renderingAttachmentInfo(depth, VK_ATTACHMENT_LOAD_OP_CLEAR, { 1.0f, 0 });

		VkRenderingInfo renderInfo{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.renderArea = renderArea,
			.layerCount = 1,
			.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size()),
			.pColorAttachments = colorAttachments.data(),
			.pDepthAttachment = &depthAttachment,
		};

		vkCmdBeginRendering(frameInfo.cmd, &renderInfo);
		{
			Tools::vk::cmdViewport(frameInfo.cmd, renderArea.extent);
			Tools::vk::cmdScissor(frameInfo.cmd, renderArea.extent);

			for (const auto& batch : frameInfo.drawBatcher.batches())
			{
				// Bind pipeline

				// Bind bindless descriptor set

				// Push constants

				// Draw call

			}
		}
		vkCmdEndRendering(frameInfo.cmd);
	}
}