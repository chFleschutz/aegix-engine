#pragma once

#include "graphics/descriptors.h"

namespace Aegix::Graphics
{
	class BaseMaterial
	{
	public:
		BaseMaterial(VulkanDevice& device) : m_device{ device } {}
		virtual ~BaseMaterial() = default;

		virtual void initialize(DescriptorSetLayout& setLayout, DescriptorPool& pool) = 0;

		VkDescriptorSet descriptorSet(int index) { return m_descriptorSets[index]; }

	protected:
		VulkanDevice& m_device;

		std::vector<VkDescriptorSet> m_descriptorSets;
	};
}
