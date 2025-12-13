#include "pch.h"
#include "instance_update_pass.h"

#include "scene/components.h"
#include "graphics/vulkan/vulkan_tools.h"
#include "graphics/draw_batch_registry.h"

#include <glm/gtx/matrix_major_storage.hpp>

namespace Aegix::Graphics
{
	InstanceUpdatePass::InstanceUpdatePass(FGResourcePool& pool)
	{
		m_instanceBuffer = pool.addBuffer("InstanceData",
			FGResource::Usage::TransferDst,
			FGBufferInfo{
				.size = sizeof(InstanceData) * MAX_INSTANCES,
				.instanceCount = MAX_FRAMES_IN_FLIGHT,
				.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				.allocFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
							  VMA_ALLOCATION_CREATE_MAPPED_BIT,
			});

		m_drawBatchBuffer = pool.addBuffer("DrawBatches",
			FGResource::Usage::TransferDst,
			FGBufferInfo{
				.size = sizeof(DrawBatchData) * DrawBatchRegistry::MAX_DRAW_BATCHES,
				.instanceCount = MAX_FRAMES_IN_FLIGHT,
				.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				.allocFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
							  VMA_ALLOCATION_CREATE_MAPPED_BIT,
			});
	}

	auto InstanceUpdatePass::info() -> FGNode::Info 
	{
		return FGNode::Info{
			.name = "Instance Update",
			.reads = {},
			.writes = { m_instanceBuffer },
		};
	}

	void InstanceUpdatePass::execute(FGResourcePool& pool, const FrameInfo& frameInfo)
	{
		// TODO: Update instance data on demand only (when scene changes)
		auto& instanceBuffer = pool.buffer(m_instanceBuffer);

		uint32_t instanceID = 0;
		auto view = frameInfo.scene.registry().view<GlobalTransform, Mesh, Material>();
		for (const auto& [entity, transform, mesh, material] : view.each())
		{
			if (instanceID >= MAX_INSTANCES)
			{
				ALOG::warn("Instance Update: Reached maximum instance count of {}", MAX_INSTANCES);
				break;
			}

			if (!mesh.staticMesh || !material.instance || !material.instance->materialTemplate())
				continue;

			const auto& matInstance = material.instance;
			const auto& matTemplate = matInstance->materialTemplate();

			// Update instance data (if needed)
			matInstance->updateParameters(frameInfo.frameIndex);

			// Shader needs both in row major (better packing)
			glm::mat4 modelMatrix = transform.matrix();
			glm::mat3 normalMatrix = glm::inverse(modelMatrix);

			auto data = instanceBuffer.buffer().mappedAs<InstanceData>(frameInfo.frameIndex);
			data[instanceID] = InstanceData{
				.modelMatrix = glm::rowMajor4(modelMatrix),
				.normalRow0 = normalMatrix[0],
				.meshHandle = mesh.staticMesh->meshDataBuffer().handle(),
				.normalRow1 = normalMatrix[1],
				// TODO: Find solution to avoid per-frame handle retrieval (this needs to be static)
				.materialHandle = matInstance->buffer().handle(frameInfo.frameIndex),
				.normalRow2 = normalMatrix[2],
				.drawBatchID = matTemplate->drawBatch(),
			};

			instanceID++;
		}

		// Update draw batch info
		auto& drawBatchBuffer = pool.buffer(m_drawBatchBuffer);
		auto drawBatchData = drawBatchBuffer.buffer().mappedAs<DrawBatchData>(frameInfo.frameIndex);
		const auto& drawBatches = frameInfo.drawBatcher.batches();
		for (size_t i = 0; i < drawBatches.size(); i++)
		{
			drawBatchData[i] = DrawBatchData{
				.instanceOffset = drawBatches[i].firstInstance,
				.instanceCount = drawBatches[i].instanceCount,
			};
		}
	}
}
