#include "pch.h"
#include "instance_update_pass.h"

#include "scene/components.h"
#include "graphics/vulkan/vulkan_tools.h"

#include <glm/gtx/matrix_major_storage.hpp>

namespace Aegix::Graphics
{
	InstanceUpdatePass::InstanceUpdatePass(FGResourcePool& pool) : 
		m_updateBuffer{ Buffer::stagingBuffer(sizeof(InstanceData) * MAX_INSTANCES, MAX_FRAMES_IN_FLIGHT) }
	{
		m_instanceBuffer = pool.addBuffer("InstanceBuffer",
			FGResource::Usage::TransferDst,
			FGBufferInfo{
				.size = sizeof(InstanceData) * MAX_INSTANCES,
			});
	}

	auto InstanceUpdatePass::info() -> FGNode::Info 
	{
		return FGNode::Info{
			.name = "Instance Update Pass",
			.reads = {},
			.writes = { m_instanceBuffer },
		};
	}

	void InstanceUpdatePass::execute(FGResourcePool& pool, const FrameInfo& frameInfo)
	{
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

			// Shader needs both in row major (better packing)
			glm::mat4 modelMatrix = glm::rowMajor4(transform.matrix());
			glm::mat3 normalMatrix = glm::inverse(modelMatrix);

			auto data = m_updateBuffer.mappedAs<InstanceData>(frameInfo.frameIndex);
			data[instanceID] = InstanceData{
				.modelMatrix = modelMatrix,
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

		// Copy updated instance data to the GPU instance buffer
		auto& instanceBuffer = pool.buffer(m_instanceBuffer);
		m_updateBuffer.flush();
		m_updateBuffer.copyTo(frameInfo.cmd, instanceBuffer, frameInfo.frameIndex, 0);
	}
}
