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
		void free(const DescriptorHandle& handle);

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

		static constexpr uint32_t SAMPLED_IMAGE_BINDING = 0;
		static constexpr uint32_t STORAGE_IMAGE_BINDING = 1;
		static constexpr uint32_t STORAGE_BUFFER_BINDING = 2;

		BindlessDescriptorSet();
		~BindlessDescriptorSet();

		auto addSampledImage(const Texture& texture) -> DescriptorHandle;
		auto addStorageImage(const Texture& texture) -> DescriptorHandle;
		auto addStorageBuffer(const Buffer& buffer) -> DescriptorHandle;

		void freeHandle(const DescriptorHandle& handle);

	private:
		auto createDescriptorPool() -> DescriptorPool;
		auto createDescriptorSetLayout() -> DescriptorSetLayout;

		DescriptorPool m_bindlessPool;
		DescriptorSetLayout m_bindlessSetLayout;
		DescriptorSet m_bindlessDescriptorSet;

		DescriptorHandleCache m_sampledImageCache{ MAX_SAMPLED_IMAGES };
		DescriptorHandleCache m_storageImageCache{ MAX_STORAGE_IMAGES };
		DescriptorHandleCache m_storageBufferCache{ MAX_STORAGE_BUFFERS };
	};
}