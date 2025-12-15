#include "pch.h"
#include "instance_update_pass.h"

#include "scene/components.h"
#include "graphics/vulkan/vulkan_tools.h"
#include "graphics/draw_batch_registry.h"
#include "core/profiler.h"

#include <glm/gtx/matrix_major_storage.hpp>

namespace Aegix::Graphics
{
	InstanceUpdatePass::InstanceUpdatePass(FGResourcePool& pool)
	{
		m_staticInstances = pool.addBuffer("StaticInstanceData",
			FGResource::Usage::TransferDst,
			FGBufferInfo{
				.size = sizeof(InstanceData) * MAX_STATIC_INSTANCES,
				.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				// TODO: Remove the host visible flag and use staging buffer for upload instead
				.allocFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
							  VMA_ALLOCATION_CREATE_MAPPED_BIT,
			});

		m_dynamicInstances = pool.addBuffer("DynamicInstanceData",
			FGResource::Usage::TransferDst,
			FGBufferInfo{
				.size = sizeof(InstanceData) * MAX_DYNAMIC_INSTANCES,
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
			.writes = { m_staticInstances, m_dynamicInstances, m_drawBatchBuffer },
		};
	}

	void InstanceUpdatePass::sceneInitialized(FGResourcePool& resources, Scene::Scene& scene)
	{
		std::vector<InstanceData> staticInstances;
		staticInstances.reserve(MAX_STATIC_INSTANCES); // TODO: use draw batcher for actual count

		uint32_t instanceID = 0;
		auto view = scene.registry().view<GlobalTransform, Mesh, Material>(entt::exclude<DynamicTag>);
		for (const auto& [entity, transform, mesh, material] : view.each())
		{
			if (instanceID >= MAX_STATIC_INSTANCES)
			{
				ALOG::warn("Instance Update: Reached maximum static instance count of {}", MAX_STATIC_INSTANCES);
				break;
			}

			if (!mesh.staticMesh || !material.instance || !material.instance->materialTemplate())
				continue;

			const auto& matInstance = material.instance;
			const auto& matTemplate = matInstance->materialTemplate();

			// Update instance data (if needed)
			matInstance->updateParameters(0);

			// Shader needs both in row major (better packing)
			glm::mat4 modelMatrix = transform.matrix();
			glm::mat3 normalMatrix = glm::inverse(modelMatrix);
			staticInstances.emplace_back(glm::rowMajor4(modelMatrix),
				normalMatrix[0], mesh.staticMesh->meshDataBuffer().handle(),
				normalMatrix[1], matInstance->buffer().handle(0),  // TODO: Would normal use frame index but data is static so its fine i guess?
				normalMatrix[2], matTemplate->drawBatch());

			instanceID++;
		}

		// Copy instance data to mapped buffer
		auto& staticBuffer = resources.buffer(m_staticInstances);
		staticBuffer.buffer().copy(staticInstances, 0);
	}

	void InstanceUpdatePass::execute(FGResourcePool& pool, const FrameInfo& frameInfo)
	{
		AGX_PROFILE_FUNCTION();

		// TODO: Update instance data on demand only (when scene changes)

		std::vector<InstanceData> dynamicInstances;
		dynamicInstances.reserve(frameInfo.drawBatcher.instanceCount()); // TODO: Differentiate static/dynamic counts

		uint32_t instanceID = 0;
		auto view = frameInfo.scene.registry().view<GlobalTransform, Mesh, Material, DynamicTag>();
		for (const auto& [entity, transform, mesh, material] : view.each())
		{
			if (instanceID >= MAX_DYNAMIC_INSTANCES)
			{
				ALOG::warn("Instance Update: Reached maximum instance count of {}", MAX_DYNAMIC_INSTANCES);
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

			dynamicInstances.emplace_back(glm::rowMajor4(modelMatrix),
				normalMatrix[0], mesh.staticMesh->meshDataBuffer().handle(),
				normalMatrix[1], matInstance->buffer().handle(frameInfo.frameIndex),
				normalMatrix[2], matTemplate->drawBatch());

			instanceID++;
		}

		// Copy instance data to mapped buffer
		auto& instanceBuffer = pool.buffer(m_dynamicInstances);
		instanceBuffer.buffer().copy(dynamicInstances, frameInfo.frameIndex);

		// Update draw batch info
		auto& drawBatchBuffer = pool.buffer(m_drawBatchBuffer);
		drawBatchBuffer.buffer().copy(frameInfo.drawBatcher.batches(), frameInfo.frameIndex);
	}
}
