#include "pch.h"
#include "descriptor_handle.h"

namespace Aegix::Graphics
{
	DescriptorHandle::DescriptorHandle(uint32_t index, Type type, Access access) :
		m_index{ index },
		m_version{ 0 },
		m_type{ static_cast<uint8_t>(type) },
		m_access{ static_cast<uint8_t>(access) }
	{
		static_assert(sizeof(DescriptorHandle) == sizeof(uint64_t), "DescriptorHandle size must be 8 bytes");
	}

	void DescriptorHandle::recycle(Type type, Access access)
	{
		AGX_ASSERT_X(isValid(), "Cannot recycle an invalid DescriptorHandle");

		m_version = m_version + 1; // Possible overflow is intentional
		m_type = static_cast<uint8_t>(type);
		m_access = static_cast<uint8_t>(access);
	}
}