#include "pch.h"
#include "descriptor_handle.h"

namespace Aegix::Graphics
{
	DescriptorHandle::DescriptorHandle(uint32_t index, Type type) :
		m_handle{ pack(index, 0, type) }
	{
		static_assert(sizeof(DescriptorHandle) == sizeof(uint32_t), "Shader expects DescriptorHandle to be 4 bytes");
	}

	auto DescriptorHandle::pack(uint32_t index, uint32_t version, Type type) -> uint32_t
	{
		AGX_ASSERT_X(index <= INDEX_MASK, "Index out of bounds for DescriptorHandle");
		AGX_ASSERT_X(version <= VERSION_MASK, "Version out of bounds for DescriptorHandle");

		return (index & INDEX_MASK) |
			((version & VERSION_MASK) << INDEX_BITS) |
			((static_cast<uint32_t>(type) & TYPE_MASK) << (INDEX_BITS + VERSION_BITS));
	}

	void DescriptorHandle::recycle(Type type)
	{
		AGX_ASSERT_X(isValid(), "Cannot recycle an invalid DescriptorHandle");

		constexpr uint32_t MAX_VERSION = (1 << VERSION_BITS);
		m_handle = pack(index(), (version() + 1) % MAX_VERSION, type);
	}
}