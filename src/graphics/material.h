#pragma once

#include "graphics/buffer.h"
#include "graphics/descriptors.h"

namespace Aegix::Graphics
{
	template<typename T>
	struct RenderSystemRef;

	class BaseMaterial
	{
	public:
		BaseMaterial(VulkanDevice& device, DescriptorSetLayout& setLayout, DescriptorPool& pool) : m_device{ device } {}
		virtual ~BaseMaterial() = default;

		VkDescriptorSet descriptorSet(int index) { return m_descriptorSets[index]; }

	protected:
		VulkanDevice& m_device;

		std::vector<std::unique_ptr<Buffer>> m_uniformBuffers;
		std::vector<VkDescriptorSet> m_descriptorSets;
	};
}
