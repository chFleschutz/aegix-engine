#pragma once

#include "graphics/descriptors.h"

namespace Aegix::Graphics
{
	class DescriptorHandle
	{
		friend class DescriptorHandleCache;

	public:
		enum class Type : uint8_t
		{
			Buffer = 0,
			Texture = 1,
			RWTexture = 2
		};

		enum class Access : uint8_t
		{
			ReadOnly = 0,
			ReadWrite = 1
		};

		static constexpr uint32_t INDEX_BITS   = 23;
		static constexpr uint32_t TYPE_BITS    = 2;
		static constexpr uint32_t ACCESS_BITS  = 1;
		static constexpr uint32_t VERSION_BITS = 6;

		static constexpr uint32_t INDEX_MASK   = (1u << INDEX_BITS) - 1;
		static constexpr uint32_t TYPE_MASK    = (1u << TYPE_BITS) - 1;
		static constexpr uint32_t ACCESS_MASK  = (1u << ACCESS_BITS) - 1;
		static constexpr uint32_t VERSION_MASK = (1u << VERSION_BITS) - 1;

		[[nodiscard]] auto index() const -> uint32_t { return m_handle & INDEX_MASK; }
		[[nodiscard]] auto type() const -> Type { return static_cast<Type>((m_handle >> INDEX_BITS) & TYPE_MASK); }
		[[nodiscard]] auto access() const -> Access { return static_cast<Access>((m_handle >> (INDEX_BITS + TYPE_BITS)) & ACCESS_MASK); }
		[[nodiscard]] auto version() const -> uint32_t { return (m_handle >> (INDEX_BITS + TYPE_BITS + ACCESS_BITS)) & VERSION_MASK; }

	private:
		DescriptorHandle(uint32_t index, Type type, Access access);

		auto createHandle(uint32_t index, Type type, Access access, uint32_t version) -> uint32_t;
		void recycle(Type type, Access access);

		// Packed as: | 6 bits version | 1 bit access | 2 bits type | 23 bits index |
		uint32_t m_handle;
	};



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