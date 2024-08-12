#pragma once

#include "graphics/descriptors.h"

namespace Aegix::Graphics
{
	class BaseMaterial
	{
	public:
		BaseMaterial(VulkanDevice& device, DescriptorSetLayout& setLayout, DescriptorPool& pool) : m_device{ device } {}
		virtual ~BaseMaterial() = default;

		VkDescriptorSet descriptorSet(int index) { return m_descriptorSets[index]; }

	protected:
		VulkanDevice& m_device;

		std::vector<VkDescriptorSet> m_descriptorSets;
	};


	template<typename T>
	struct RenderSystemRef;
}
