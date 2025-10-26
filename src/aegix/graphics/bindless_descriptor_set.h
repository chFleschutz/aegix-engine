#pragma once

#include "graphics/descriptors.h"

namespace Aegix::Graphics
{
	struct DescriptorHandle
	{
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

		DescriptorHandle(uint32_t index, Type type, Access access, uint32_t version) :
			handle{
				((version & VERSION_MASK) << (INDEX_BITS + TYPE_BITS + ACCESS_BITS)) |
				((static_cast<uint32_t>(access) & ACCESS_MASK) << (INDEX_BITS + TYPE_BITS)) |
				((static_cast<uint32_t>(type) & TYPE_MASK) << INDEX_BITS) |
				((index & INDEX_MASK) << 0) 
			}
		{
			static_assert(sizeof(DescriptorHandle) == sizeof(uint32_t), "DescriptorHandle size must be 4 bytes");
			static_assert(INDEX_BITS + TYPE_BITS + ACCESS_BITS + VERSION_BITS == 32, "DescriptorHandle bit allocation must sum to 32 bits");

			AGX_ASSERT(index <= INDEX_MASK, "DescriptorHandle index out of bounds");
			AGX_ASSERT(static_cast<uint32_t>(type) <= TYPE_MASK, "DescriptorHandle type out of bounds");
			AGX_ASSERT(static_cast<uint32_t>(access) <= ACCESS_MASK, "DescriptorHandle access out of bounds");
			AGX_ASSERT(version <= VERSION_MASK, "DescriptorHandle version out of bounds");
		}

		[[nodiscard]] auto index() const -> uint32_t { return handle & INDEX_MASK; }
		[[nodiscard]] auto type() const -> Type { return static_cast<Type>((handle >> INDEX_BITS) & TYPE_MASK); }
		[[nodiscard]] auto access() const -> Access { return static_cast<Access>((handle >> (INDEX_BITS + TYPE_BITS)) & ACCESS_MASK); }
		[[nodiscard]] auto version() const -> uint32_t { return (handle >> (INDEX_BITS + TYPE_BITS + ACCESS_BITS)) & VERSION_MASK; }

		// Packed as: | 6 bits version | 1 bit access | 2 bits type | 23 bits index |
		uint32_t handle;
	};

	class BindlessDescriptorSet
	{
	public:
		static constexpr uint32_t MAX_TEXTURES = 16 * 1024;			// 16K textures
		static constexpr uint32_t MAX_STORAGE_IMAGES = 1 * 1024;	// 1K storage images
		static constexpr uint32_t MAX_STORAGE_BUFFERS = 16 * 1024;  // 16K storage buffers

		BindlessDescriptorSet();
		~BindlessDescriptorSet();

		auto allocateHandle(const Texture& texture) -> DescriptorHandle
		{
			// Allocate a new descriptor handle for the given texture
			return DescriptorHandle{ 0, DescriptorHandle::Type::Buffer, DescriptorHandle::Access::ReadOnly, 0 };
		}

		auto allocateHandle(const Buffer& buffer) -> DescriptorHandle
		{
			// Allocate a new descriptor handle for the given buffer
			return DescriptorHandle{ 0, DescriptorHandle::Type::Buffer, DescriptorHandle::Access::ReadOnly, 0 };
		}

		void freeHandle(DescriptorHandle handle)
		{
			// Free the given descriptor handle
		}

	private:
		auto createDescriptorPool() -> DescriptorPool;


		DescriptorPool m_globalPool;
		// DescriptorSetLayout m_globalSetLayout;
		// DescriptorSet m_globalDescriptorSet;
		// std::vector<DescriptorHandle> m_freeHandles;
	};
}