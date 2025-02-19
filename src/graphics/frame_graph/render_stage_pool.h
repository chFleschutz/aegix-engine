#pragma once

#include "graphics/descriptors.h"
#include "graphics/frame_info.h"
#include "graphics/systems/render_system.h"
#include "graphics/uniform_buffer.h"
#include "scene/entity.h"

#include <array>
#include <memory>
#include <vector>

namespace Aegix::Graphics
{
	enum class RenderStageType
	{
		Geometry,
		Count
	};

	struct RenderStage
	{
		std::vector<std::unique_ptr<RenderSystem>> renderSystems;
		std::unique_ptr<DescriptorSetLayout> descriptorSetLayout;
		std::unique_ptr<DescriptorSet> descriptorSet;
		std::unique_ptr<UniformBuffer> ubo;
	};

	class RenderStagePool
	{
	public:
		RenderStagePool() = default;

		void createRenderStages(VulkanDevice& device, DescriptorPool& pool)
		{
			createGeometryStage(device, pool);
		}

		auto renderStage(RenderStageType type) -> RenderStage& { return m_renderStages[static_cast<size_t>(type)]; }

		void update(FrameInfo& frameInfo)
		{
			updateGeometryStage(frameInfo);
		}

	private:
		void createGeometryStage(VulkanDevice& device, DescriptorPool& pool)
		{
			auto& stage = renderStage(RenderStageType::Geometry);

			stage.descriptorSetLayout = DescriptorSetLayout::Builder(device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.build();

			stage.ubo = std::make_unique<UniformBuffer>(device, GlobalUbo{});

			stage.descriptorSet = DescriptorSet::Builder(device, pool, *stage.descriptorSetLayout)
				.addBuffer(0, *stage.ubo)
				.build();
		}

		void updateGeometryStage(FrameInfo& frameInfo)
		{
			auto& stage = renderStage(RenderStageType::Geometry);

			auto& camera = frameInfo.scene.camera().getComponent<Component::Camera>();
			camera.aspect = frameInfo.aspectRatio;

			GlobalUbo ubo{
				.projection = camera.projectionMatrix,
				.view = camera.viewMatrix,
				.inverseView = camera.inverseViewMatrix
			};

			stage.ubo->setData(frameInfo.frameIndex, ubo);
		}

		std::array<RenderStage, static_cast<size_t>(RenderStageType::Count)> m_renderStages;
	};
}