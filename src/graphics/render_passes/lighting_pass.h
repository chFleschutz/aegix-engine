#pragma once

#include "graphics/frame_graph/frame_graph.h"
#include "graphics/frame_graph/frame_graph_blackboard.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/pipeline.h"
#include "graphics/vulkan_tools.h"

namespace Aegix::Graphics
{
	struct Lighting
	{
		struct AmbientLight
		{
			glm::vec4 color;
		};

		struct DirectionalLight
		{
			glm::vec4 direction;
			glm::vec4 color;
		};

		struct PointLight
		{
			glm::vec4 position;
			glm::vec4 color;
		};

		glm::vec4 cameraPosition;
		AmbientLight ambient;
		DirectionalLight directional;
		std::array<PointLight, MAX_POINT_LIGHTS> pointLights;
		int32_t pointLightCount;
	};


	class LightingPass : public FrameGraphRenderPass
	{
	public:
		LightingPass(VulkanDevice& device, DescriptorPool& pool)
		{
			m_descriptorSetLayout = DescriptorSetLayout::Builder(device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
				.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
				.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
				.addBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
				.addBinding(4, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
				.addBinding(5, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
				.addBinding(6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
				.build();

			m_descriptorSet = std::make_unique<DescriptorSet>(pool, *m_descriptorSetLayout);

			m_ubo = std::make_unique<UniformBufferData<Lighting>>(device);

			m_pipelineLayout = PipelineLayout::Builder(device)
				.addDescriptorSetLayout(*m_descriptorSetLayout)
				.build();

			m_pipeline = Pipeline::ComputeBuilder(device, *m_pipelineLayout)
				.setShaderStage(SHADER_DIR "lighting.comp.spv")
				.build();
		}

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override
		{
			m_position = builder.add({ "Position", 
				FrameGraphResourceType::Reference, 
				FrameGraphResourceUsage::Compute
				});
			m_normal = builder.add({ "Normal",
				FrameGraphResourceType::Reference,
				FrameGraphResourceUsage::Compute
				});
			m_albedo = builder.add({ "Albedo",
				FrameGraphResourceType::Reference,
				FrameGraphResourceUsage::Compute
				});
			m_arm = builder.add({ "ARM",
				FrameGraphResourceType::Reference,
				FrameGraphResourceUsage::Compute
				});
			m_emissive = builder.add({ "Emissive",
				FrameGraphResourceType::Reference,
				FrameGraphResourceUsage::Compute
				});
			m_sceneColor = builder.add({ "SceneColor",
				FrameGraphResourceType::Texture,
				FrameGraphResourceUsage::Compute,
				FrameGraphResourceTextureInfo{
					.format = VK_FORMAT_R16G16B16A16_SFLOAT,
					.extent = { 0, 0 },
					.resizePolicy = ResizePolicy::SwapchainRelative
					} 
				});

			return FrameGraphNodeCreateInfo{
				.name = "Lighting Pass",
				.inputs = { m_position, m_normal, m_albedo, m_arm, m_emissive },
				.outputs = { m_sceneColor }
			};
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
			VkCommandBuffer cmd = frameInfo.commandBuffer;

			updateLightingUBO(frameInfo);

			DescriptorWriter{ *m_descriptorSetLayout }
				.writeImage(0, resources.texture(m_sceneColor))
				.writeImage(1, resources.texture(m_position))
				.writeImage(2, resources.texture(m_normal))
				.writeImage(3, resources.texture(m_albedo))
				.writeImage(4, resources.texture(m_arm))
				.writeImage(5, resources.texture(m_emissive))
				.writeBuffer(6, m_ubo->descriptorBufferInfo(frameInfo.frameIndex))
				.build(m_descriptorSet->descriptorSet(frameInfo.frameIndex));

			m_pipeline->bind(cmd);
			m_descriptorSet->bind(cmd, *m_pipelineLayout, frameInfo.frameIndex, VK_PIPELINE_BIND_POINT_COMPUTE);

			auto extent = frameInfo.swapChainExtent;
			uint32_t groupCountX = (extent.width + 15) / 16;
			uint32_t groupCountY = (extent.height + 15) / 16;
			vkCmdDispatch(cmd, groupCountX, groupCountY, 1);
		}

	private:
		void updateLightingUBO(const FrameInfo& frameInfo)
		{
			Lighting ubo{};

			ubo.cameraPosition = glm::vec4(frameInfo.scene.camera().getComponent<Component::Transform>().location, 1.0f);

			ubo.ambient = {
				.color = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)
			};

			ubo.directional = {
				.direction = glm::vec4(glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)), 0.0f),
				.color = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)
			};

			int32_t lighIndex = 0;
			auto view = frameInfo.scene.viewEntities<Aegix::Component::Transform, Aegix::Component::PointLight>();
			for (auto&& [entity, transform, pointLight] : view.each())
			{
				assert(lighIndex < MAX_POINT_LIGHTS && "Point lights exceed maximum number of point lights");
				ubo.pointLights[lighIndex] = Lighting::PointLight{
					.position = glm::vec4(transform.location, 1.0f),
					.color = glm::vec4(pointLight.color, pointLight.intensity)
				};
				lighIndex++;

				assert(lighIndex < 4);
			}
			ubo.pointLightCount = lighIndex;

			m_ubo->setData(frameInfo.frameIndex, ubo);
		}

	private:
		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_position;
		FrameGraphResourceHandle m_normal;
		FrameGraphResourceHandle m_albedo;
		FrameGraphResourceHandle m_arm;
		FrameGraphResourceHandle m_emissive;

		std::unique_ptr<Pipeline> m_pipeline;
		std::unique_ptr<PipelineLayout> m_pipelineLayout;
		std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
		std::unique_ptr<DescriptorSet> m_descriptorSet;
		std::unique_ptr<UniformBufferData<Lighting>> m_ubo;
	};
}