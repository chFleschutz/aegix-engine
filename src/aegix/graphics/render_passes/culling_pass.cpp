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

		m_visibleDrawSet = pool.addBuffer("Visible Draw Set",
			FGResource::Usage::ComputeWriteStorage,
			FGBufferInfo{
				.size = sizeof(uint32_t) * static_cast<size_t>(m_drawBatcher.instanceCount() * INSTANCE_OVERALLOCATION)
			});

		// TODO: Need to declare more resources here (instance buffer, draw batche list, indirect draw buffer, etc)
	}

	auto CullingPass::info() -> FGNode::Info
	{
		return FGNode::Info{
			.name = "Culling",
			.reads = {},
			.writes = { m_visibleDrawSet },
		};
	}

	void CullingPass::execute(FGResourcePool& pool, const FrameInfo& frameInfo)
	{
		return; //TODO: Temporarily disable culling pass

		CullingPushConstants push{
			// TODO: Fill in push constants
		};

		m_pipeline.bind(frameInfo.cmd);
		m_pipeline.bindDescriptorSet(frameInfo.cmd, 0, Engine::renderer().bindlessDescriptorSet().descriptorSet());
		m_pipeline.pushConstants(frameInfo.cmd, VK_SHADER_STAGE_COMPUTE_BIT, &push, sizeof(CullingPushConstants));

		// TODO: Find a cleaner way here
		constexpr uint32_t workgroupSize = 64;
		uint32_t groupCountX = (m_drawBatcher.instanceCount() + workgroupSize - 1) / workgroupSize;
		vkCmdDispatch(frameInfo.cmd, groupCountX, 1, 1);
	}
}