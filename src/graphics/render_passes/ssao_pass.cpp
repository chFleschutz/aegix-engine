#include "ssao_pass.h"

#include "graphics/vulkan_tools.h"
#include "math/interpolation.h"
#include "math/random.h"

#include <imgui.h>

namespace Aegix::Graphics
{
	SSAOPass::SSAOPass(VulkanDevice& device, DescriptorPool& pool)
	{
		m_descriptorSetLayout = DescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(5, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
			.build();

		m_descriptorSet = std::make_unique<DescriptorSet>(pool, *m_descriptorSetLayout);

		m_pipelineLayout = PipelineLayout::Builder(device)
			.addDescriptorSetLayout(*m_descriptorSetLayout)
			.build();

		m_pipeline = Pipeline::ComputeBuilder(device, *m_pipelineLayout)
			.setShaderStage(SHADER_DIR "ssao.comp.spv")
			.build();

		m_uniforms = std::make_unique<Buffer>(device, sizeof(SSAOUniforms), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		m_uniforms->map();

		// Generate random samples
		auto& gen = Random::generator();
		std::uniform_real_distribution dis(0.0f, 1.0f);
		std::vector<glm::vec4> samples;
		samples.reserve(SAMPLE_COUNT);
		for (uint32_t i = 0; i < SAMPLE_COUNT; i++)
		{
			glm::vec4 sample{
				dis(gen) * 2.0f - 1.0f,
				dis(gen) * 2.0f - 1.0f,
				dis(gen),
				0.0f
			};
			sample = glm::normalize(sample);
			sample *= dis(gen);

			float scale = static_cast<float>(i) / static_cast<float>(SAMPLE_COUNT);
			sample *= Math::lerp(0.1f, 1.0f, scale * scale);
			samples.emplace_back(sample);
		}

		m_ssaoSamples = std::make_unique<Buffer>(device, sizeof(glm::vec4), static_cast<uint32_t>(samples.size()),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		m_ssaoSamples->map();
		m_ssaoSamples->writeToBuffer(samples.data());
		m_ssaoSamples->unmap();

		// Generate noise texture
		std::vector<glm::vec2> noise(NOISE_SIZE * NOISE_SIZE);
		for (auto& n : noise)
		{
			n.x = dis(gen) * 2.0f - 1.0f;
			n.y = dis(gen) * 2.0f - 1.0f;
		}

		m_ssaoNoise = std::make_unique<Texture>(device, NOISE_SIZE, NOISE_SIZE, VK_FORMAT_R16G16_UNORM,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL | VK_IMAGE_USAGE_SAMPLED_BIT);
		m_ssaoNoise->fill(noise.data(), sizeof(glm::vec2) * noise.size());
		m_ssaoNoise->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	auto SSAOPass::createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo
	{
		m_position = builder.add(FrameGraphResourceCreateInfo{
				.name = "Position",
				.type = FrameGraphResourceType::Reference,
				.usage = FrameGraphResourceUsage::Sampled
			});

		m_normal = builder.add(FrameGraphResourceCreateInfo{
				.name = "Normal",
				.type = FrameGraphResourceType::Reference,
				.usage = FrameGraphResourceUsage::Sampled
			});

		m_ssao = builder.add(FrameGraphResourceCreateInfo{
				.name = "SSAO",
				.type = FrameGraphResourceType::Texture,
				.usage = FrameGraphResourceUsage::Compute,
				.info = FrameGraphResourceTextureInfo{
					.format = VK_FORMAT_R8_UNORM,
					.extent = { 0, 0 },
					.resizePolicy = ResizePolicy::SwapchainRelative
				}
			});

		return FrameGraphNodeCreateInfo{
			.name = "SSAO",
			.inputs = { m_position, m_normal },
			.outputs = { m_ssao }
		};
	}

	void SSAOPass::execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo)
	{
		// Update push constants
		auto& camera = frameInfo.scene.mainCamera().component<Camera>();
		m_uniformData.view = camera.viewMatrix;
		m_uniformData.projection = camera.projectionMatrix;
		m_uniformData.noiseScale.x = m_uniformData.noiseScale.y * camera.aspect;
		m_uniforms->writeToBuffer(&m_uniformData);

		VkCommandBuffer cmd = frameInfo.commandBuffer;

		DescriptorWriter{ *m_descriptorSetLayout }
			.writeImage(0, resources.texture(m_ssao))
			.writeImage(1, resources.texture(m_position))
			.writeImage(2, resources.texture(m_normal))
			.writeImage(3, *m_ssaoNoise)
			.writeBuffer(4, *m_ssaoSamples)
			.writeBuffer(5, *m_uniforms)
			.build(m_descriptorSet->descriptorSet(frameInfo.frameIndex));

		m_pipeline->bind(cmd);
		m_descriptorSet->bind(cmd, *m_pipelineLayout, frameInfo.frameIndex, VK_PIPELINE_BIND_POINT_COMPUTE);

		Tools::vk::cmdDispatch(cmd, frameInfo.swapChainExtent, { 16, 16 });
	}

	void SSAOPass::drawUI()
	{
		ImGui::DragFloat("Noise Scale", &m_uniformData.noiseScale.y, 0.01f, 0.0f, 100.0f);
		ImGui::DragFloat("Radius", &m_uniformData.radius, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Bias", &m_uniformData.bias, 0.001f, 0.0f, 1.0f);
		ImGui::DragFloat("Power", &m_uniformData.power, 0.01f, 0.0f, 10.0f);
	}
}
