#include "pch.h"
#include "descriptor_handle.h"

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
}