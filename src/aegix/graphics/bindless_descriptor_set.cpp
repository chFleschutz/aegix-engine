#include "bindless_descriptor_set.h"

#include "graphics/vulkan/vulkan_context.h"

namespace Aegix::Graphics
{
	BindlessDescriptorSet::BindlessDescriptorSet() :
		m_bindlessPool{ createDescriptorPool() },
		m_bindlessSetLayout{ createDescriptorSetLayout() },
		m_bindlessDescriptorSet{ m_bindlessSetLayout, m_bindlessPool }
	{
	}

	BindlessDescriptorSet::~BindlessDescriptorSet()
	{
	}

	auto BindlessDescriptorSet::createDescriptorPool() -> DescriptorPool
	{
		const auto& limits = VulkanContext::device().properties().limits;
		return DescriptorPool::Builder()
			.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, std::min(MAX_TEXTURES, limits.maxDescriptorSetSampledImages))
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, std::min(MAX_STORAGE_IMAGES, limits.maxDescriptorSetStorageImages))
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, std::min(MAX_STORAGE_BUFFERS, limits.maxDescriptorSetStorageBuffers))
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)
			.setMaxSets(1)
			.build();
	}

	auto BindlessDescriptorSet::createDescriptorSetLayout() -> DescriptorSetLayout
	{
		const auto& limits = VulkanContext::device().properties().limits;
		return DescriptorSetLayout::Builder()
			.addBinding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL, std::min(MAX_TEXTURES, limits.maxDescriptorSetSampledImages))
			.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_ALL, std::min(MAX_STORAGE_IMAGES, limits.maxDescriptorSetStorageImages))
			.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL, std::min(MAX_STORAGE_BUFFERS, limits.maxDescriptorSetStorageBuffers))
			.setBindingFlags(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)
			.setFlags(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)
			.build();
	}
}
