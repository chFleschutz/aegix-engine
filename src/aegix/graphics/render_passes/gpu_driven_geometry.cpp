#include "pch.h"
#include "gpu_driven_geometry.h"

#include "graphics/vulkan/vulkan_tools.h"
#include "scene/components.h"

#include <glm/gtx/matrix_major_storage.hpp>

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

		m_staticInstanceData = pool.addReference("StaticInstanceData",
			FGResource::Usage::ComputeReadStorage);

		m_dynamicInstanceData = pool.addReference("DynamicInstanceData",
			FGResource::Usage::ComputeReadStorage);

		m_drawBatches = pool.addReference("DrawBatches",
			FGResource::Usage::ComputeReadStorage);

		m_indirectDrawCommands = pool.addReference("IndirectDrawCommands",
			FGResource::Usage::IndirectBuffer);

		m_indirectDrawCounts = pool.addReference("IndirectDrawCounts",
			FGResource::Usage::IndirectBuffer);

		m_cameraData = pool.addReference("CameraData",
			FGResource::Usage::ComputeReadUniform);
	}

	auto GPUDrivenGeometry::info() -> FGNode::Info
	{
		return FGNode::Info{
			.name = "GPU Driven Geometry",
			.reads = { m_staticInstanceData, m_dynamicInstanceData, m_visibleInstances, 
				m_indirectDrawCommands, m_indirectDrawCounts, m_cameraData },
			.writes = { m_position, m_normal, m_albedo, m_arm, m_emissive, m_depth }
		};
	}

	void GPUDrivenGeometry::execute(FGResourcePool& pool, const FrameInfo& frameInfo)
	{
		VkRect2D renderArea{
			.offset = { 0, 0 },
			.extent = frameInfo.swapChainExtent
		};

		auto colorAttachments = std::array{
			Tools::renderingAttachmentInfo(pool.texture(m_position), VK_ATTACHMENT_LOAD_OP_CLEAR),
			Tools::renderingAttachmentInfo(pool.texture(m_normal), VK_ATTACHMENT_LOAD_OP_CLEAR),
			Tools::renderingAttachmentInfo(pool.texture(m_albedo), VK_ATTACHMENT_LOAD_OP_CLEAR),
			Tools::renderingAttachmentInfo(pool.texture(m_arm), VK_ATTACHMENT_LOAD_OP_CLEAR),
			Tools::renderingAttachmentInfo(pool.texture(m_emissive), VK_ATTACHMENT_LOAD_OP_CLEAR)
		};
		auto depthAttachment = Tools::renderingAttachmentInfo(pool.texture(m_depth), VK_ATTACHMENT_LOAD_OP_CLEAR, { 1.0f, 0 });

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

			auto& cameraData = pool.buffer(m_cameraData);
			auto& staticInstanceData = pool.buffer(m_staticInstanceData);
			auto& dynamicInstanceData = pool.buffer(m_dynamicInstanceData);
			auto& visibleInstances = pool.buffer(m_visibleInstances);
			auto& indirectDrawCommands = pool.buffer(m_indirectDrawCommands);
			auto& indirectDrawCounts = pool.buffer(m_indirectDrawCounts);
			for (const auto& batch : frameInfo.drawBatcher.batches())
			{
				PushConstant pushConstants{
					.cameraData = cameraData.handle(frameInfo.frameIndex),
					.staticInstances = staticInstanceData.handle(),
					.dynamicInstances = dynamicInstanceData.handle(frameInfo.frameIndex),
					.visibility = visibleInstances.handle(),
					.batchFirstID = batch.firstInstance,
					.batchSize = batch.instanceCount,
					.staticCount = frameInfo.drawBatcher.staticInstanceCount(),
					.dynamicCount = frameInfo.drawBatcher.dynamicInstanceCount()
				};
				AGX_ASSERT_X(pushConstants.cameraData.isValid(), "GPU Driven Geometry Pass: Invalid camera data handle in push constants");
				batch.materialTemplate->bind(frameInfo.cmd);
				batch.materialTemplate->bindBindlessSet(frameInfo.cmd);
				batch.materialTemplate->pushConstants(frameInfo.cmd, &pushConstants, sizeof(PushConstant));

				vkCmdDrawMeshTasksIndirectCountEXT(frameInfo.cmd,
					indirectDrawCommands.buffer(),
					sizeof(VkDrawMeshTasksIndirectCommandEXT) * batch.firstInstance,
					indirectDrawCounts.buffer(),
					sizeof(uint32_t) * batch.batchID,
					batch.instanceCount,
					sizeof(VkDrawMeshTasksIndirectCommandEXT)
				);
			}
		}
		vkCmdEndRendering(frameInfo.cmd);
	}
}