#include "bindless_descriptor_set.h"

#include "graphics/vulkan/vulkan_context.h"

namespace Aegix::Graphics
{
	BindlessDescriptorSet::BindlessDescriptorSet() :
		m_globalPool{ createDescriptorPool() }
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
}
