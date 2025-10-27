#include "bindless_descriptor_set.h"

#include "graphics/vulkan/vulkan_context.h"

namespace Aegix::Graphics
{
	DescriptorHandle::DescriptorHandle(uint32_t index, Type type, Access access) :
		m_handle{ createHandle(index, type, access, 0) }
	{
		static_assert(sizeof(DescriptorHandle) == sizeof(uint32_t), "DescriptorHandle size must be 4 bytes");
		static_assert(INDEX_BITS + TYPE_BITS + ACCESS_BITS + VERSION_BITS == 32, "DescriptorHandle bit allocation must sum to 32 bits");
	}

	auto DescriptorHandle::createHandle(uint32_t index, Type type, Access access, uint32_t version) -> uint32_t
	{
		AGX_ASSERT_X(index <= INDEX_MASK, "DescriptorHandle index out of bounds");
		AGX_ASSERT_X(static_cast<uint32_t>(type) <= TYPE_MASK, "DescriptorHandle type out of bounds");
		AGX_ASSERT_X(static_cast<uint32_t>(access) <= ACCESS_MASK, "DescriptorHandle access out of bounds");
		AGX_ASSERT_X(version <= VERSION_MASK, "DescriptorHandle version out of bounds");

		return
			((version & VERSION_MASK) << (INDEX_BITS + TYPE_BITS + ACCESS_BITS)) |
			((static_cast<uint32_t>(access) & ACCESS_MASK) << (INDEX_BITS + TYPE_BITS)) |
			((static_cast<uint32_t>(type) & TYPE_MASK) << INDEX_BITS) |
			((index & INDEX_MASK) << 0);
	}

	void DescriptorHandle::recycle(Type type, Access access)
	{
		AGX_ASSERT_X(isValid(), "Cannot recycle an invalid DescriptorHandle");

		constexpr uint32_t maxVersion = VERSION_MASK + 1;
		m_handle = createHandle(index(), type, access, (version() + 1) % maxVersion);
	}


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

	void DescriptorHandleCache::free(const DescriptorHandle& handle)
	{
		AGX_ASSERT_X(handle.isValid(), "Cannot free an invalid DescriptorHandle!");
		AGX_ASSERT_X(handle.index() < m_capacity, "DescriptorHandle index out of bounds!");
		m_availableHandles.emplace_back(handle);
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

	auto BindlessDescriptorSet::addSampledImage(const Texture& texture) -> DescriptorHandle
	{
		auto handle = m_sampledImageCache.fetch(DescriptorHandle::Type::Texture, DescriptorHandle::Access::ReadOnly);
		auto textureInfo = texture.descriptorImageInfo();

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

	auto BindlessDescriptorSet::addStorageImage(const Texture& texture) -> DescriptorHandle
	{
		auto handle = m_storageImageCache.fetch(DescriptorHandle::Type::RWTexture, DescriptorHandle::Access::ReadWrite);
		auto textureInfo = texture.descriptorImageInfo();

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

	auto BindlessDescriptorSet::addStorageBuffer(const Buffer& buffer) -> DescriptorHandle
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

	void BindlessDescriptorSet::freeHandle(const DescriptorHandle& handle)
	{
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
