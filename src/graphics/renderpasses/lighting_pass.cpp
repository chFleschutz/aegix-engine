#include "lighting_pass.h"

#include "scene/components.h"
#include "scene/entity.h"

namespace Aegix::Graphics
{
	LightingPass::LightingPass(VulkanDevice& device, DescriptorPool& pool)
	{
		m_globalSetLayout = DescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		m_globalUBO = std::make_unique<UniformBuffer<GlobalUbo>>(device);

		m_globalDescriptorSet = DescriptorSet::Builder(device, pool, *m_globalSetLayout)
			.addBuffer(0, *m_globalUBO)
			.build();
	}

	void LightingPass::render(FrameInfo& frameInfo)
	{
		// TODO: Update aspect ration only on resize
		auto& camera = frameInfo.scene.camera().getComponent<Component::Camera>();
		camera.aspect = frameInfo.aspectRatio;

		// TODO: Bind global descriptor set here instead of in each render system
		frameInfo.globalDescriptorSet = m_globalDescriptorSet->descriptorSet(frameInfo.frameIndex);

		updateGlobalUBO(frameInfo);

		beginRenderPass(frameInfo);

		for (auto&& [_, system] : m_renderSystemCollection)
		{
			system->render(frameInfo);
		}

		endRenderPass(frameInfo);
	}

	void LightingPass::updateGlobalUBO(const FrameInfo& frameInfo)
	{
		auto& camera = frameInfo.scene.camera().getComponent<Component::Camera>();

		GlobalUbo ubo{};
		ubo.projection = camera.projectionMatrix;
		ubo.view = camera.viewMatrix;
		ubo.inverseView = camera.inverseViewMatrix;

		int lighIndex = 0;
		auto view = frameInfo.scene.viewEntities<Aegix::Component::Transform, Aegix::Component::PointLight>();
		for (auto&& [entity, transform, pointLight] : view.each())
		{
			assert(lighIndex < GlobalLimits::MAX_LIGHTS && "Point lights exceed maximum number of point lights");
			ubo.pointLights[lighIndex].position = glm::vec4(transform.location, 1.0f);
			ubo.pointLights[lighIndex].color = glm::vec4(pointLight.color, pointLight.intensity);
			lighIndex++;
		}
		ubo.numLights = lighIndex;

		m_globalUBO->setData(frameInfo.frameIndex, ubo);
	}

	void LightingPass::beginRenderPass(FrameInfo& frameInfo)
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = frameInfo.swapChainRenderPass;
		renderPassInfo.framebuffer = frameInfo.swapChainFramebuffer;

		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = { frameInfo.swapChainExtent };

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(frameInfo.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(frameInfo.swapChainExtent.width);
		viewport.height = static_cast<float>(frameInfo.swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(frameInfo.commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{ { 0, 0 }, frameInfo.swapChainExtent };
		vkCmdSetScissor(frameInfo.commandBuffer, 0, 1, &scissor);
	}

	void LightingPass::endRenderPass(FrameInfo& frameInfo)
	{
		vkCmdEndRenderPass(frameInfo.commandBuffer);
	}
}