#include "pch.h"
#include "instance_update_pass.h"

#include "scene/components.h"

#include <glm/gtx/matrix_major_storage.hpp>

namespace Aegix::Graphics
{
	InstanceUpdatePass::InstanceUpdatePass(FGResourcePool& pool)
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
		auto& instanceBuffer = pool.buffer(m_instanceBuffer);
		instanceBuffer.map();

		auto view = frameInfo.scene.registry().view<GlobalTransform, Mesh, Material>();
		for (const auto& [entity, transform, mesh, material] : view.each())
		{
			if (!mesh.staticMesh || !material.instance || !material.instance->materialTemplate())
				continue;

			// TODO: use proper mapping
			uint32_t instanceID = static_cast<uint32_t>(entity);

			// Both in row major
			auto modelMatrix = glm::rowMajor4(transform.matrix());
			auto normalMatrix = glm::mat3(glm::inverse(modelMatrix));

			auto data = instanceBuffer.mappedAs<InstanceData>();
			data[instanceID] = InstanceData{
				.modelMatrix = glm::mat4x3(modelMatrix),
				.normalRow0 = normalMatrix[0],
				.meshHandle = mesh.staticMesh->meshDataBuffer().handle(),
				.normalRow1 = normalMatrix[1],
				//.materialHandle = material.instance->materialTemplate()->handle(),
				.normalRow2 = normalMatrix[2],
			};
		}

		instanceBuffer.flush();
		instanceBuffer.unmap();
	}
}
