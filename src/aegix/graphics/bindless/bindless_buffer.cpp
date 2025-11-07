#include "pch.h"
#include "bindless_buffer.h"

#include "engine.h"

namespace Aegix::Graphics
{
	BindlessBuffer::BindlessBuffer(const Buffer::CreateInfo& bufferInfo) : 
		m_buffer{ bufferInfo }
	{
		auto& bindlessSet = Engine::renderer().bindlessDescriptorSet();
		if (bufferInfo.usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
		{
			m_handle = bindlessSet.allocateStorageBuffer(m_buffer.descriptorBufferInfo());
		}
		else if (bufferInfo.usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
		{
			m_handle = bindlessSet.allocateUniformBuffer(m_buffer.descriptorBufferInfo());
		}
		else
		{
			AGX_ASSERT_X(false, "BindlessBuffer only supports storage and uniform buffers");
		}
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

	BindlessFrameBuffer::BindlessFrameBuffer(const Buffer::CreateInfo& bufferInfo) :
		m_buffer{ bufferInfo }
	{
		AGX_ASSERT_X(bufferInfo.instanceCount == MAX_FRAMES_IN_FLIGHT,
			"BindlessBufferArray requires instanceCount to be equal to MAX_FRAMES_IN_FLIGHT");

		auto& bindlessSet = Engine::renderer().bindlessDescriptorSet();
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (bufferInfo.usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
			{
				m_handles[i] = bindlessSet.allocateStorageBuffer(m_buffer.descriptorBufferInfoFor(i));
			}
			else if (bufferInfo.usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
			{
				m_handles[i] = bindlessSet.allocateUniformBuffer(m_buffer.descriptorBufferInfoFor(i));
			}
			else
			{
				AGX_ASSERT_X(false, "BindlessFrameBuffer only supports storage and uniform buffers");
			}
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