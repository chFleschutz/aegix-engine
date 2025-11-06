#pragma once

#include "graphics/bindless/descriptor_handle.h"
#include "graphics/descriptors.h"
#include "graphics/resources/buffer.h"
#include "graphics/resources/texture.h"

namespace Aegix::Graphics
{
	class DescriptorHandleCache
	{
	public:
		DescriptorHandleCache(uint32_t capacity);

		auto fetch(DescriptorHandle::Type type, DescriptorHandle::Access access) -> DescriptorHandle;
		void free(DescriptorHandle& handle);

	private:
		uint32_t m_capacity;
		uint32_t m_nextIndex{ 0 };
		std::vector<DescriptorHandle> m_availableHandles;
	};



	class BindlessDescriptorSet
	{
	public:
		static constexpr uint32_t MAX_SAMPLED_IMAGES = 16 * 1024;	// 16K sampled images
		static constexpr uint32_t MAX_STORAGE_IMAGES = 1 * 1024;	// 1K  storage images
		static constexpr uint32_t MAX_STORAGE_BUFFERS = 16 * 1024;  // 16K storage buffers
		static constexpr uint32_t MAX_UNIFORM_BUFFERS = 16 * 1024;  // 16K uniform buffers

		static constexpr uint32_t SAMPLED_IMAGE_BINDING = 1;
		static constexpr uint32_t STORAGE_IMAGE_BINDING = 3;
		static constexpr uint32_t STORAGE_BUFFER_BINDING = 6;
		static constexpr uint32_t UNIFORM_BUFFER_BINDING = 8;

		BindlessDescriptorSet();
		~BindlessDescriptorSet();

		[[nodiscard]] auto descriptorSet() const -> const DescriptorSet& { return m_bindlessDescriptorSet; }
		[[nodiscard]] auto layout() const -> const DescriptorSetLayout& { return m_bindlessSetLayout; }

		auto allocateSampledImage(const Texture& texture) -> DescriptorHandle;
		auto allocateStorageImage(const Texture& texture) -> DescriptorHandle;
		auto allocateStorageBuffer(const Buffer& buffer) -> DescriptorHandle;
		auto allocateUniformBuffer(const Buffer& buffer) -> DescriptorHandle;

		void freeHandle(DescriptorHandle& handle);

	private:
		auto createDescriptorPool() -> DescriptorPool;
		auto createDescriptorSetLayout() -> DescriptorSetLayout;
		void writeSet(uint32_t binding, uint32_t index, VkDescriptorType type,
			const VkDescriptorImageInfo* imageInfo, const VkDescriptorBufferInfo* bufferInfo);

		DescriptorPool m_bindlessPool;
		DescriptorSetLayout m_bindlessSetLayout;
		DescriptorSet m_bindlessDescriptorSet;

		DescriptorHandleCache m_sampledImageCache{ MAX_SAMPLED_IMAGES };
		DescriptorHandleCache m_storageImageCache{ MAX_STORAGE_IMAGES };
		DescriptorHandleCache m_storageBufferCache{ MAX_STORAGE_BUFFERS };
		DescriptorHandleCache m_uniformBufferCache{ MAX_UNIFORM_BUFFERS };
	};
}