#pragma once

#include "graphics/frame_graph/frame_graph.h"
#include "graphics/frame_graph/frame_graph_blackboard.h"
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

	struct LightingResources
	{
		FrameGraphResourceID sceneColor;
		std::unique_ptr<Pipeline> pipeline;
		std::unique_ptr<PipelineLayout> pipelineLayout;
		std::unique_ptr<DescriptorSetLayout> descriptorSetLayout;
		std::unique_ptr<DescriptorSet> descriptorSet;
		std::unique_ptr<Sampler> sampler;
		std::unique_ptr<UniformBufferData<LightingUBO>> ubo;
	};

	struct LightingData
	{
		FrameGraphResourceID sceneColor;
	};

	class LightingPass
	{
	public:
		LightingPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard, FrameGraphResourceID sceneColor)
		{
			const auto& resources = frameGraph.resourcePool();
			const auto& gBuffer = blackboard.get<GBufferData>();
			const auto& renderer = blackboard.get<RendererData>();

			const auto& lightingResources = frameGraph.addPass<LightingResources>("Lighting",
				[&](FrameGraph::Builder& builder, LightingResources& data)
				{
					data.sceneColor = builder.declareWrite(sceneColor);

					builder.declareRead(gBuffer.position);
					builder.declareRead(gBuffer.normal);
					builder.declareRead(gBuffer.albedo);
					builder.declareRead(gBuffer.arm);
					builder.declareRead(gBuffer.emissive);
					builder.declareRead(gBuffer.depth);

					const auto& position = resources.getTexture(gBuffer.position);
					const auto& normal = resources.getTexture(gBuffer.normal);
					const auto& albedo = resources.getTexture(gBuffer.albedo);
					const auto& arm = resources.getTexture(gBuffer.arm);
					const auto& emissive = resources.getTexture(gBuffer.emissive);

					data.sampler = std::make_unique<Sampler>(renderer.device);

					data.ubo = std::make_unique<UniformBufferData<LightingUBO>>(renderer.device);

					data.descriptorSetLayout = DescriptorSetLayout::Builder(renderer.device)
						.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
						.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
						.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
						.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
						.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
						.addBinding(5, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
						.build();

					data.descriptorSet = DescriptorSet::Builder(renderer.device, renderer.pool, *data.descriptorSetLayout)
						.addTexture(0, position.texture, *data.sampler)
						.addTexture(1, normal.texture, *data.sampler)
						.addTexture(2, albedo.texture, *data.sampler)
						.addTexture(3, arm.texture, *data.sampler)
						.addTexture(4, emissive.texture, *data.sampler)
						.addBuffer(5, *data.ubo)
						.build();

					data.pipelineLayout = PipelineLayout::Builder(renderer.device)
						.addDescriptorSetLayout(*data.descriptorSetLayout)
						.build();

					data.pipeline = Pipeline::Builder(renderer.device, *data.pipelineLayout)
						.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, SHADER_DIR "deferred.vert.spv")
						.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "deferred.frag.spv")
						.addColorAttachment(VK_FORMAT_B8G8R8A8_SRGB)
						.setDepthAttachment(VK_FORMAT_D32_SFLOAT)
						.setVertexAttributeDescriptions({}) // Clear default vertex attributes
						.setVertexBindingDescriptions({}) // Clear default vertex binding
						.build();
				},
				[](const LightingResources& data, const FrameGraphResourcePool& resources, const FrameInfo& frameInfo)
				{
					updateLighting(frameInfo, const_cast<LightingResources&>(data));

					VkCommandBuffer commandBuffer = frameInfo.commandBuffer;

					// Begin Rendering
					VkRenderingAttachmentInfo colorAttachment{};
					colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
					colorAttachment.imageView = frameInfo.swapChainColor;
					colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					colorAttachment.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };

					VkRenderingAttachmentInfo depthAttachment{};
					depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
					depthAttachment.imageView = frameInfo.swapChainDepth;
					depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					depthAttachment.clearValue.depthStencil = { 1.0f, 0 };

					VkRect2D renderArea{};
					renderArea.offset = { 0,0 };
					renderArea.extent = frameInfo.swapChainExtend;

					VkRenderingInfo renderInfo{};
					renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
					renderInfo.renderArea = renderArea;
					renderInfo.layerCount = 1;
					renderInfo.colorAttachmentCount = 1;
					renderInfo.pColorAttachments = &colorAttachment;
					renderInfo.pDepthAttachment = &depthAttachment;

					vkCmdBeginRendering(commandBuffer, &renderInfo);

					VkViewport viewport{};
					viewport.x = 0.0f;
					viewport.y = 0.0f;
					viewport.width = static_cast<float>(frameInfo.swapChainExtend.width);
					viewport.height = static_cast<float>(frameInfo.swapChainExtend.height);
					viewport.minDepth = 0.0f;
					viewport.maxDepth = 1.0f;

					vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

					VkRect2D scissor{};
					scissor.offset = { 0, 0 };
					scissor.extent = frameInfo.swapChainExtend;

					vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

					// Bind Pipeline
					data.pipeline->bind(commandBuffer);

					// Bind Descriptor Set
					vkCmdBindDescriptorSets(
						commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						data.pipelineLayout->pipelineLayout(),
						0, 1,
						&data.descriptorSet->descriptorSet(frameInfo.frameIndex),
						0, nullptr
					);

					// Draw Fullscreen Triangle
					vkCmdDraw(commandBuffer, 3, 1, 0, 0);

					// End Rendering
					vkCmdEndRendering(commandBuffer);
				});

			blackboard.add<LightingData>(lightingResources.sceneColor);
		}

		static void updateLighting(const FrameInfo& frameInfo, LightingResources& data)
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

			data.ubo->setData(frameInfo.frameIndex, ubo);
		}
	};
}