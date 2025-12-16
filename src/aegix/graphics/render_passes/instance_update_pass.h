#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/bindless/descriptor_handle.h"

namespace Aegix::Graphics
{
	// TODO: Maybe use SoA for better cache use (instead of packed AoS)
	struct alignas(16) InstanceData
	{
		glm::mat3x4 modelMatrix;
		glm::vec3 normalRow0;
		DescriptorHandle meshHandle;
		glm::vec3 normalRow1;
		DescriptorHandle materialHandle;
		glm::vec3 normalRow2;
		uint32_t drawBatchID;
	};

	struct DrawBatchData
	{
		uint32_t instanceOffset;
		uint32_t instanceCount;
	};

	class InstanceUpdatePass : public FGRenderPass
	{
	public:
		static constexpr size_t MAX_STATIC_INSTANCES = 10'000;
		static constexpr size_t MAX_DYNAMIC_INSTANCES = 1'000;

		InstanceUpdatePass(FGResourcePool& pool);
		auto info() -> FGNode::Info override;
		virtual void sceneInitialized(FGResourcePool& resources, Scene::Scene& scene) override;
		virtual void execute(FGResourcePool& pool, const FrameInfo& frameInfo) override;

	private:
		FGResourceHandle m_staticInstances;
		FGResourceHandle m_dynamicInstances;
		FGResourceHandle m_drawBatchBuffer;
	};
}