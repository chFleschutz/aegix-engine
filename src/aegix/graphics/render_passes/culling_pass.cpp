#include "pch.h"
#include "culling_pass.h"

#include "engine.h"
#include "graphics/vulkan/vulkan_tools.h"

namespace Aegix::Graphics
{
	CullingPass::CullingPass(FGResourcePool& pool, DrawBatchRegistry& batcher)
		: m_drawBatcher{ batcher }
	{
		m_pipeline = Pipeline::ComputeBuilder{}
			// TODO: Maybe add convienience method to add bindless layout
			.addDescriptorSetLayout(Engine::renderer().bindlessDescriptorSet().layout())
			.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(CullingPushConstants))
			.setShaderStage(SHADER_DIR "culling.slang.spv")
			.build();

		m_instanceBuffer = pool.addReference("InstanceData",
			FGResource::Usage::ComputeReadStorage);

		m_drawBatchBuffer = pool.addReference("DrawBatchBuffer",
			FGResource::Usage::ComputeReadStorage);

		m_visibleIndices = pool.addBuffer("VisibleInstances",
			FGResource::Usage::ComputeWriteStorage,
			FGBufferInfo{
				.size = sizeof(uint32_t) * static_cast<size_t>(m_drawBatcher.instanceCount() * INSTANCE_OVERALLOCATION)
			});

		m_visibleCounts = pool.addBuffer("VisibleInstanceCounts",
			FGResource::Usage::ComputeWriteStorage,
			FGBufferInfo{
				.size = sizeof(uint32_t) * m_drawBatcher.batchCount(),
				.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT
			});
	}

	auto CullingPass::info() -> FGNode::Info
	{
		return FGNode::Info{
			.name = "Culling",
			.reads = { m_instanceBuffer },
			.writes = { m_visibleIndices, m_visibleCounts },
		};
	}

	void CullingPass::execute(FGResourcePool& pool, const FrameInfo& frameInfo)
	{
		// Clear visible counts buffer
		auto& visibleCountBuffer = pool.buffer(m_visibleCounts);
		vkCmdFillBuffer(frameInfo.cmd, visibleCountBuffer.buffer(), 0, visibleCountBuffer.buffer().bufferSize(), 0);
		
		CullingPushConstants push{
			.instanceBuffer = pool.buffer(m_instanceBuffer).handle(),
			.drawBatchBuffer = pool.buffer(m_drawBatchBuffer).handle(),
			.visibilityBuffer = pool.buffer(m_visibleIndices).handle(),
			.visibleCountBuffer = pool.buffer(m_visibleCounts).handle(),
			.instanceCount = m_drawBatcher.instanceCount(),
		};

		m_pipeline.bind(frameInfo.cmd);
		m_pipeline.bindDescriptorSet(frameInfo.cmd, 0, Engine::renderer().bindlessDescriptorSet());
		m_pipeline.pushConstants(frameInfo.cmd, VK_SHADER_STAGE_COMPUTE_BIT, push);

		Tools::vk::cmdDispatch(frameInfo.cmd, m_drawBatcher.instanceCount(), WORKGROUP_SIZE);
	}
}