#include "pch.h"
#include "bindless_buffer.h"

#include "engine.h"

namespace Aegis::Graphics
{
	static auto allocateBindlessHandle(const Buffer& buffer) -> DescriptorHandle
	{
		auto& bindlessSet = Engine::renderer().bindlessDescriptorSet();
		if (buffer.usage() & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
		{
			return bindlessSet.allocateStorageBuffer(buffer.descriptorBufferInfo());
		}
		else if (buffer.usage() & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
		{
			return bindlessSet.allocateUniformBuffer(buffer.descriptorBufferInfo());
		}
		else
		{
			AGX_ASSERT_X(false, "allocateBindlessHandle only supports storage and uniform buffers");
			return DescriptorHandle{};
		}
	}

	static auto allocateBindlessHandle(const Buffer& buffer, uint32_t index) -> DescriptorHandle
	{
		auto& bindlessSet = Engine::renderer().bindlessDescriptorSet();
		if (buffer.usage() & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
		{
			return bindlessSet.allocateStorageBuffer(buffer.descriptorBufferInfoFor(index));
		}
		else if (buffer.usage() & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
		{
			return bindlessSet.allocateUniformBuffer(buffer.descriptorBufferInfoFor(index));
		}
		else
		{
			AGX_ASSERT_X(false, "allocateBindlessHandles only supports storage and uniform buffers");
			return DescriptorHandle{};
		}
	}


	BindlessBuffer::BindlessBuffer(const Buffer::CreateInfo& bufferInfo) : 
		m_buffer{ bufferInfo },
		m_handle{ allocateBindlessHandle(m_buffer) }
	{
	}

	BindlessBuffer::BindlessBuffer(BindlessBuffer&& other) noexcept : 
		m_buffer{ std::move(other.m_buffer) },
		m_handle{ other.m_handle }
	{
		other.m_handle.invalidate();
	}

	BindlessBuffer::~BindlessBuffer()
	{
		Engine::renderer().bindlessDescriptorSet().freeHandle(m_handle);
	}

	auto BindlessBuffer::operator=(BindlessBuffer&& other) noexcept -> BindlessBuffer&
	{
		if (this != &other)
		{
			Engine::renderer().bindlessDescriptorSet().freeHandle(m_handle);
			m_buffer = std::move(other.m_buffer);
			m_handle = other.m_handle;
			other.m_handle.invalidate();
		}
		return *this;
	}


	BindlessMultiBuffer::BindlessMultiBuffer(const Buffer::CreateInfo& bufferInfo) :
		m_buffer{ bufferInfo }
	{
		m_handles.resize(bufferInfo.instanceCount);
		for (uint32_t i = 0; i < bufferInfo.instanceCount; i++)
		{
			m_handles[i] = allocateBindlessHandle(m_buffer, i);
		}
	}

	BindlessMultiBuffer::BindlessMultiBuffer(BindlessMultiBuffer&& other) noexcept :
		m_buffer{ std::move(other.m_buffer) },
		m_handles{ std::move(other.m_handles) }
	{
		other.m_handles.clear();
	}

	BindlessMultiBuffer::~BindlessMultiBuffer()
	{
		auto& bindlessSet = Engine::renderer().bindlessDescriptorSet();
		for (auto& handle : m_handles)
		{
			bindlessSet.freeHandle(handle);
		}
	}

	auto BindlessMultiBuffer::operator=(BindlessMultiBuffer&& other) noexcept -> BindlessMultiBuffer&
	{
		if (this != &other)
		{
			auto& bindlessSet = Engine::renderer().bindlessDescriptorSet();
			for (auto& handle : m_handles)
			{
				bindlessSet.freeHandle(handle);
			}
			m_buffer = std::move(other.m_buffer);
			m_handles = std::move(other.m_handles);
			other.m_handles.clear();
		}
		return *this;
	}


	BindlessFrameBuffer::BindlessFrameBuffer(const Buffer::CreateInfo& bufferInfo) :
		m_buffer{ bufferInfo }
	{
		AGX_ASSERT_X(bufferInfo.instanceCount == MAX_FRAMES_IN_FLIGHT,
			"BindlessBufferArray requires instanceCount to be equal to MAX_FRAMES_IN_FLIGHT");

		auto& bindlessSet = Engine::renderer().bindlessDescriptorSet();
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_handles[i] = allocateBindlessHandle(m_buffer, static_cast<uint32_t>(i));
		}
	}

	BindlessFrameBuffer::~BindlessFrameBuffer()
	{
		auto& bindlessSet = Engine::renderer().bindlessDescriptorSet();
		for (auto& handle : m_handles)
		{
			bindlessSet.freeHandle(handle);
		}
	}

	auto BindlessFrameBuffer::handle(size_t index) const -> DescriptorHandle
	{
		AGX_ASSERT_X(index < MAX_FRAMES_IN_FLIGHT, "Requested handle index exceeds MAX_FRAMES_IN_FLIGHT");
		return m_handles[index];
	}
}