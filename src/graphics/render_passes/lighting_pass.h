#pragma once

#include "graphics/frame_graph/frame_graph.h"
#include "graphics/frame_graph/frame_graph_blackboard.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/pipeline.h"

namespace Aegix::Graphics
{
	struct LightingUBO
	{
		glm::vec4 cameraPosition;

		struct AmbientLight
		{
			glm::vec4 color;
		};
		AmbientLight ambient;

		struct DirectionalLight
		{
			glm::vec4 direction;
			glm::vec4 color;
		};
		DirectionalLight directional;

		struct PointLight
		{
			glm::vec4 position;
			glm::vec4 color;
		};
		std::array<PointLight, GlobalLimits::MAX_LIGHTS> pointLights;
		int32_t pointLightCount;
	};


	class LightingPass : public FrameGraphRenderPass
	{
	public:
		LightingPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard, 
			FrameGraphResourceHandle position, FrameGraphResourceHandle normal, FrameGraphResourceHandle albedo,
			FrameGraphResourceHandle arm, FrameGraphResourceHandle emissive, FrameGraphResourceHandle depth,
			FrameGraphResourceHandle sceneColor)
			: m_position{ position }, m_normal{ normal }, m_albedo{ albedo }, m_arm{ arm }, m_emissive{ emissive }, 
			m_depth{ depth }, m_sceneColor{ sceneColor }
		{
			const auto& resources = frameGraph.resourcePool();
			const auto& renderer = blackboard.get<RendererData>();

			const auto& positionTex = resources.texture(position);
			const auto& normalTex = resources.texture(normal);
			const auto& albedoTex = resources.texture(albedo);
			const auto& armTex = resources.texture(arm);
			const auto& emissiveTex = resources.texture(emissive);

			m_ubo = std::make_unique<UniformBufferData<LightingUBO>>(renderer.device);

			m_descriptorSetLayout = DescriptorSetLayout::Builder(renderer.device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(5, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.build();

			m_descriptorSet = DescriptorSet::Builder(renderer.device, renderer.pool, *m_descriptorSetLayout)
				.addTexture(0, positionTex.texture)
				.addTexture(1, normalTex.texture)
				.addTexture(2, albedoTex.texture)
				.addTexture(3, armTex.texture)
				.addTexture(4, emissiveTex.texture)
				.addBuffer(5, *m_ubo)
				.build();

			m_pipelineLayout = PipelineLayout::Builder(renderer.device)
				.addDescriptorSetLayout(*m_descriptorSetLayout)
				.build();

			m_pipeline = Pipeline::Builder(renderer.device, *m_pipelineLayout)
				.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, SHADER_DIR "deferred.vert.spv")
				.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "deferred.frag.spv")
				.addColorAttachment(VK_FORMAT_B8G8R8A8_SRGB)
				.setDepthAttachment(VK_FORMAT_D32_SFLOAT)
				.setVertexAttributeDescriptions({}) // Clear default vertex attributes
				.setVertexBindingDescriptions({}) // Clear default vertex binding
				.build();
		}

		virtual auto createInfo() -> FrameGraphNodeCreateInfo override
		{
			FrameGraphNodeCreateInfo info{};
			info.name = "Lighting Pass";
			info.inputs = { m_position, m_normal, m_albedo, m_arm, m_emissive, m_depth };
			info.outputs = { m_sceneColor };
			return info;
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
			VkCommandBuffer commandBuffer = frameInfo.commandBuffer;

			updateLightingUBO(frameInfo);

			auto positionInfo = resources.texture(m_position).texture.descriptorImageInfo();
			auto normalInfo = resources.texture(m_normal).texture.descriptorImageInfo();
			auto albedoInfo = resources.texture(m_albedo).texture.descriptorImageInfo();
			auto armInfo = resources.texture(m_arm).texture.descriptorImageInfo();
			auto emissiveInfo = resources.texture(m_emissive).texture.descriptorImageInfo();
			auto uboInfo = m_ubo->descriptorBufferInfo(frameInfo.frameIndex);

			DescriptorWriter{ *m_descriptorSetLayout }
				.writeImage(0, &positionInfo)
				.writeImage(1, &normalInfo)
				.writeImage(2, &albedoInfo)
				.writeImage(3, &armInfo)
				.writeImage(4, &emissiveInfo)
				.writeBuffer(5, &uboInfo)
				.build(m_descriptorSet->descriptorSet(frameInfo.frameIndex));

			VkRenderingAttachmentInfo colorAttachment = Tools::renderingAttachmentInfo(frameInfo.swapChainColor,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 0.0f, 1.0f });

			VkRenderingAttachmentInfo depthAttachment = Tools::renderingAttachmentInfo(frameInfo.swapChainDepth,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR, { 1.0f, 0 });

			VkRenderingInfo renderInfo{};
			renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
			renderInfo.renderArea.offset = { 0, 0 };
			renderInfo.renderArea.extent = frameInfo.swapChainExtend;
			renderInfo.layerCount = 1;
			renderInfo.colorAttachmentCount = 1;
			renderInfo.pColorAttachments = &colorAttachment;
			renderInfo.pDepthAttachment = &depthAttachment;

			vkCmdBeginRendering(commandBuffer, &renderInfo);

			Tools::vk::cmdViewport(commandBuffer, frameInfo.swapChainExtend);
			Tools::vk::cmdScissor(commandBuffer, frameInfo.swapChainExtend);

			m_pipeline->bind(commandBuffer);

			Tools::vk::cmdBindDescriptorSet(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				*m_pipelineLayout, m_descriptorSet->descriptorSet(frameInfo.frameIndex));

			// Draw Fullscreen Triangle
			vkCmdDraw(commandBuffer, 3, 1, 0, 0);

			vkCmdEndRendering(commandBuffer);
		}

	private:
		void updateLightingUBO(const FrameInfo& frameInfo)
		{
			LightingUBO ubo{};

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
				assert(lighIndex < GlobalLimits::MAX_LIGHTS && "Point lights exceed maximum number of point lights");
				ubo.pointLights[lighIndex] = LightingUBO::PointLight{
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
		FrameGraphResourceHandle m_depth;
		std::unique_ptr<Pipeline> m_pipeline;
		std::unique_ptr<PipelineLayout> m_pipelineLayout;
		std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
		std::unique_ptr<DescriptorSet> m_descriptorSet;
		std::unique_ptr<UniformBufferData<LightingUBO>> m_ubo;
	};
}