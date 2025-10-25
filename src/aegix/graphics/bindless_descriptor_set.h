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

		DescriptorHandle(uint32_t index, Type type, Access access, uint32_t version) :
			handle{ (version << 26) | (static_cast<uint32_t>(access) << 25) | (static_cast<uint32_t>(type) << 23) | index}
		{
		}

		uint32_t handle;
	};

	class BindlessDescriptorSet
	{
	public:
		BindlessDescriptorSet()
		{
			// Create bindless descriptor pool
			// Create global descriptor set layout
			// Allocate global descriptor set
		}

		~BindlessDescriptorSet()
		{
			// Cleanup
		}

		auto allocateHandle(const Texture& texture) -> DescriptorHandle
		{
			// Allocate a new descriptor handle for the given texture
			return DescriptorHandle{ 0, 0 };
		}

		auto allocateHandle(const Buffer& buffer) -> DescriptorHandle
		{
			// Allocate a new descriptor handle for the given buffer
			return DescriptorHandle{ 0, 0 };
		}

		void freeHandle(DescriptorHandle handle)
		{
			// Free the given descriptor handle
		}

	private:
		// DescriptorPool m_globalPool;
		// DescriptorSetLayout m_globalSetLayout;
		// DescriptorSet m_globalDescriptorSet;
		// std::vector<DescriptorHandle> m_freeHandles;
	};
}