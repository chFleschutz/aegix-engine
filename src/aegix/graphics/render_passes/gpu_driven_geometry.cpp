#include "pch.h"
#include "gpu_driven_geometry.h"

#include "graphics/vulkan/vulkan_tools.h"
#include "scene/components.h"

#include <glm/gtx/matrix_major_storage.hpp>

namespace Aegix::Graphics
{
	GPUDrivenGeometry::GPUDrivenGeometry(FGResourcePool& pool) :
		m_global{ Buffer::uniformBuffer(sizeof(GlobalUBO), MAX_FRAMES_IN_FLIGHT) }
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

		m_drawBatches = pool.addReference("DrawBatches",
			FGResource::Usage::ComputeReadStorage);
	}

	auto GPUDrivenGeometry::info() -> FGNode::Info 
	{
		return FGNode::Info{
			.name = "GPU Driven Geometry",
			.reads = { m_instanceData, m_visibleInstances },
			.writes = { m_position, m_normal, m_albedo, m_arm, m_emissive, m_depth }
		};
	}

	void GPUDrivenGeometry::execute(FGResourcePool& pool, const FrameInfo& frameInfo)
	{
		updateUBO(frameInfo);

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

			for (const auto& batch : frameInfo.drawBatcher.batches())
			{
				PushConstants pushConstants{
					.global = m_global.handle(frameInfo.frameIndex),
					.instance = pool.buffer(m_instanceData).handle(),
					.visibility = pool.buffer(m_visibleInstances).handle(),
					.firstInstance = batch.firstInstance,
					.instanceCount = batch.instanceCount,
				};

				batch.materialTemplate->bind(frameInfo.cmd);
				batch.materialTemplate->bindBindlessSet(frameInfo.cmd);
				batch.materialTemplate->pushConstants(frameInfo.cmd, &pushConstants, sizeof(PushConstants));
				batch.materialTemplate->drawInstanced(frameInfo.cmd, batch.instanceCount);
			}
		}
		vkCmdEndRendering(frameInfo.cmd);
	}

	void GPUDrivenGeometry::updateUBO(const FrameInfo& frameInfo)
	{
		Scene::Entity mainCamera = frameInfo.scene.mainCamera();
		if (!mainCamera)
			return;

		// TODO: Don't update this here (move to renderer or similar)
		auto& camera = mainCamera.get<Camera>();
		camera.aspect = frameInfo.aspectRatio;

		auto data = m_global.buffer().data<GlobalUBO>(frameInfo.frameIndex);
		data->projection = glm::rowMajor4(camera.projectionMatrix);
		data->view = glm::rowMajor4(camera.viewMatrix);
		data->inverseView = glm::rowMajor4(camera.inverseViewMatrix);
	}
}