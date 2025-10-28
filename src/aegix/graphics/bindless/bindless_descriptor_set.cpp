#include "bindless_descriptor_set.h"

#include "graphics/vulkan/vulkan_context.h"

namespace Aegix::Graphics
{
	// DescriptorHandleCache --------------------

	DescriptorHandleCache::DescriptorHandleCache(uint32_t capacity) :
		m_capacity{ capacity }
	{
	}

	auto DescriptorHandleCache::fetch(DescriptorHandle::Type type, DescriptorHandle::Access access) -> DescriptorHandle
	{
		if (!m_availableHandles.empty())
		{
			DescriptorHandle handle = m_availableHandles.back();
			m_availableHandles.pop_back();
			handle.recycle(type, access);
			return handle;
		}

		AGX_ASSERT_X(m_nextIndex < m_capacity, "DescriptorHandleCache capacity exceeded!");
		return DescriptorHandle{ m_nextIndex++, type, access };
	}

	void DescriptorHandleCache::free(DescriptorHandle& handle)
	{
		if (!handle.isValid())
			return;

		AGX_ASSERT_X(handle.index() < m_capacity, "DescriptorHandle index out of bounds!");
		m_availableHandles.emplace_back(handle);
		handle.invalidate();
	}


	// BindlessDescriptorSet --------------------

	BindlessDescriptorSet::BindlessDescriptorSet() :
		m_bindlessPool{ createDescriptorPool() },
		m_bindlessSetLayout{ createDescriptorSetLayout() },
		m_bindlessDescriptorSet{ m_bindlessSetLayout, m_bindlessPool }
	{
	}

	BindlessDescriptorSet::~BindlessDescriptorSet()
	{
	}

	auto BindlessDescriptorSet::allocateSampledImage(const Texture& texture) -> DescriptorHandle
	{
		auto handle = m_sampledImageCache.fetch(DescriptorHandle::Type::Texture, DescriptorHandle::Access::ReadOnly);
		auto textureInfo = texture.descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		VkWriteDescriptorSet write{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = m_bindlessDescriptorSet,
			.dstBinding = SAMPLED_IMAGE_BINDING,
			.dstArrayElement = handle.index(),
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.pImageInfo = &textureInfo,
		};
		vkUpdateDescriptorSets(VulkanContext::device(), 1, &write, 0, nullptr);

		return handle;
	}

	auto BindlessDescriptorSet::allocateStorageImage(const Texture& texture) -> DescriptorHandle
	{
		auto handle = m_storageImageCache.fetch(DescriptorHandle::Type::RWTexture, DescriptorHandle::Access::ReadWrite);
		auto textureInfo = texture.descriptorImageInfo(VK_IMAGE_LAYOUT_GENERAL);

		VkWriteDescriptorSet write{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = m_bindlessDescriptorSet,
			.dstBinding = STORAGE_IMAGE_BINDING,
			.dstArrayElement = handle.index(),
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.pImageInfo = &textureInfo,
		};
		vkUpdateDescriptorSets(VulkanContext::device(), 1, &write, 0, nullptr);

		return handle;
	}

	auto BindlessDescriptorSet::allocateStorageBuffer(const Buffer& buffer) -> DescriptorHandle
	{
		auto handle = m_storageBufferCache.fetch(DescriptorHandle::Type::Buffer, DescriptorHandle::Access::ReadWrite);
		auto bufferInfo = buffer.descriptorInfo();

		VkWriteDescriptorSet write{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = m_bindlessDescriptorSet,
			.dstBinding = STORAGE_BUFFER_BINDING,
			.dstArrayElement = handle.index(),
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pBufferInfo = &bufferInfo,
		};
		vkUpdateDescriptorSets(VulkanContext::device(), 1, &write, 0, nullptr);

		return handle;
	}

	void BindlessDescriptorSet::freeHandle(DescriptorHandle& handle)
	{
		if (!handle.isValid())
			return;

		switch (handle.type())
		{
		case DescriptorHandle::Type::Texture:
			m_sampledImageCache.free(handle);
			break;
		case DescriptorHandle::Type::RWTexture:
			m_storageImageCache.free(handle);
			break;
		case DescriptorHandle::Type::Buffer:
			m_storageBufferCache.free(handle);
			break;
		default:
			AGX_ASSERT_X(false, "Unknown DescriptorHandle type!");
			break;
		}
	}

	auto BindlessDescriptorSet::createDescriptorPool() -> DescriptorPool
	{
		const auto& limits = VulkanContext::device().properties().limits;
		return DescriptorPool::Builder()
			.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, std::min(MAX_SAMPLED_IMAGES, limits.maxDescriptorSetSampledImages))
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
			.addBinding(SAMPLED_IMAGE_BINDING, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL,
				std::min(MAX_SAMPLED_IMAGES, limits.maxDescriptorSetSampledImages))
			.addBinding(STORAGE_IMAGE_BINDING, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_ALL,
				std::min(MAX_STORAGE_IMAGES, limits.maxDescriptorSetStorageImages))
			.addBinding(STORAGE_BUFFER_BINDING, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL,
				std::min(MAX_STORAGE_BUFFERS, limits.maxDescriptorSetStorageBuffers))
			.setBindingFlags(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)
			.setFlags(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)
			.build();
	}
}
