#include "pch.h"
#include "buffer.h"

#include "graphics/vulkan/vulkan_context.h"
#include "graphics/vulkan/vulkan_tools.h"

namespace Aegix::Graphics
{
	auto Buffer::createUniformBuffer(VkDeviceSize size, uint32_t instanceCount) -> Buffer
	{
		AGX_ASSERT_X(size > 0, "Cannot create uniform buffer of size 0");
		AGX_ASSERT_X(instanceCount > 0, "Cannot create uniform buffer with 0 instances");
		Buffer::CreateInfo info{
			.instanceSize = size,
			.instanceCount = instanceCount,
			.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			.allocFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
			.minOffsetAlignment = VulkanContext::device().properties().limits.minUniformBufferOffsetAlignment
		};
		return Buffer{ info };
	}

	auto Buffer::createStorageBuffer(VkDeviceSize size, uint32_t instanceCount) -> Buffer
	{
		AGX_ASSERT_X(size > 0, "Cannot create storage buffer of size 0");
		AGX_ASSERT_X(instanceCount > 0, "Cannot create storage buffer with 0 instances");
		Buffer::CreateInfo info{
			.instanceSize = size,
			.instanceCount = instanceCount,
			.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.allocFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
			.minOffsetAlignment = VulkanContext::device().properties().limits.minStorageBufferOffsetAlignment
		};
		return Buffer{ info };
	}

	auto Buffer::createVertexBuffer(VkDeviceSize size, uint32_t instanceCount, VkBufferUsageFlags otherUsage) -> Buffer
	{
		AGX_ASSERT_X(size > 0, "Cannot create vertex buffer of size 0");
		AGX_ASSERT_X(instanceCount > 0, "Cannot create vertex buffer with 0 instances");
		Buffer::CreateInfo info{
			.instanceSize = size,
			.instanceCount = instanceCount,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.allocFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
		};
		info.usage |= otherUsage;
		return Buffer{ info };
	}

	auto Buffer::createIndexBuffer(VkDeviceSize size, uint32_t instanceCount) -> Buffer
	{
		AGX_ASSERT_X(size > 0, "Cannot create index buffer of size 0");
		AGX_ASSERT_X(instanceCount > 0, "Cannot create index buffer with 0 instances");
		Buffer::CreateInfo info{
			.instanceSize = size,
			.instanceCount = instanceCount,
			.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.allocFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
		};
		return Buffer{ info };
	}

	auto Buffer::createStagingBuffer(VkDeviceSize size) -> Buffer
	{
		AGX_ASSERT_X(size > 0, "Cannot create staging buffer of size 0");
		Buffer::CreateInfo info{
			.instanceSize = size,
			.instanceCount = 1,
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			.allocFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
		};
		return Buffer{ info };
	}



	Buffer::Buffer(const CreateInfo& info) :
		m_instanceSize{ info.instanceSize },
		m_instanceCount{ info.instanceCount },
		m_usage{ info.usage }
	{
		m_alignmentSize = computeAlignment(info.instanceSize, info.minOffsetAlignment);
		m_bufferSize = m_alignmentSize * m_instanceCount;
		VulkanContext::device().createBuffer(m_buffer, m_allocation, m_bufferSize, m_usage, info.allocFlags, VMA_MEMORY_USAGE_AUTO);

		VmaAllocationInfo allocInfo;
		vmaGetAllocationInfo(VulkanContext::device().allocator(), m_allocation, &allocInfo);
		m_mapped = allocInfo.pMappedData;
	}

	Buffer::Buffer(Buffer&& other) noexcept
	{
		moveFrom(std::move(other));
	}

	Buffer::~Buffer()
	{
		destroy();
	}

	auto Buffer::operator=(Buffer&& other) noexcept -> Buffer&
	{
		if (this != &other)
		{
			destroy();
			moveFrom(std::move(other));
		}
		return *this;
	}

	auto Buffer::descriptorBufferInfo(VkDeviceSize size, VkDeviceSize offset) const -> VkDescriptorBufferInfo
	{
		AGX_ASSERT_X((size == VK_WHOLE_SIZE && offset == 0) || (offset + size <= m_bufferSize), 
			"Requested descriptor buffer info exceeds buffer size");
		return VkDescriptorBufferInfo{ m_buffer, offset, size };
	}

	auto Buffer::descriptorBufferInfoFor(uint32_t index) const -> VkDescriptorBufferInfo
	{
		AGX_ASSERT_X(index < m_instanceCount, "Requested descriptor buffer info index exceeds instance count");
		return descriptorBufferInfo(m_alignmentSize, index * m_alignmentSize);
	}

	void Buffer::map()
	{
		AGX_ASSERT_X(!m_mapped, "Buffer is already mapped");
		VK_CHECK(vmaMapMemory(VulkanContext::device().allocator(), m_allocation, &m_mapped));
	}

	void Buffer::unmap()
	{
		if (m_mapped)
		{
			vmaUnmapMemory(VulkanContext::device().allocator(), m_allocation);
			m_mapped = nullptr;
		}
	}

	void Buffer::write(const void* data)
	{
		write(data, m_bufferSize, 0);
	}

	void Buffer::write(const void* data, VkDeviceSize size, VkDeviceSize offset)
	{
		AGX_ASSERT_X(data, "Data pointer is null");
		AGX_ASSERT_X(m_mapped, "Called write on buffer before map");

		memcpy(static_cast<uint8_t*>(m_mapped) + offset, data, size);
		flush(size, offset);
	}

	void Buffer::writeToIndex(const void* data, uint32_t index)
	{
		AGX_ASSERT_X(data, "Data pointer is null");
		AGX_ASSERT_X(m_mapped, "Called write on buffer before map");
		AGX_ASSERT_X(index < m_instanceCount, "Requested write index exceeds instance count");

		memcpy(static_cast<uint8_t*>(m_mapped) + (index * m_alignmentSize), data, m_instanceSize);
		flushIndex(index);
	}

	void Buffer::singleWrite(const void* data)
	{
		AGX_ASSERT_X(data, "Data pointer is null");
		if (m_instanceCount == 1)
		{
			singleWrite(data, m_instanceSize, 0);
		}
		else // Copy data to all instances
		{
			map();
			for (uint32_t i = 0; i < m_instanceCount; i++)
			{
				memcpy(static_cast<uint8_t*>(m_mapped) + (i * m_alignmentSize), data, m_instanceSize);
			}
			flush(m_bufferSize, 0);
			unmap();
		}
	}

	void Buffer::singleWrite(const void* data, VkDeviceSize size, VkDeviceSize offset)
	{
		AGX_ASSERT_X(data, "Data pointer is null");
		AGX_ASSERT_X((size == VK_WHOLE_SIZE && offset == 0) || (offset + size <= m_bufferSize), 
			"Single write exceeds buffer size");
		VK_CHECK(vmaCopyMemoryToAllocation(VulkanContext::device().allocator(), data, m_allocation, offset, size));
	}

	void Buffer::flush(VkDeviceSize size, VkDeviceSize offset)
	{
		AGX_ASSERT_X(m_mapped, "Called flush on buffer before map");
		AGX_ASSERT_X((size == VK_WHOLE_SIZE && offset == 0) || (offset + size <= m_bufferSize), 
			"Flush range exceeds buffer size");
		VK_CHECK(vmaFlushAllocation(VulkanContext::device().allocator(), m_allocation, offset, size));
	}

	void Buffer::flushIndex(uint32_t index)
	{
		AGX_ASSERT_X(index < m_instanceCount, "Requested flush index exceeds instance count");
		flush(m_alignmentSize, index * m_alignmentSize);
	}

	void Buffer::upload(const void* data, VkDeviceSize size)
	{
		// TODO: Use a global shared staging buffer (creating a new one for each upload is inefficient)
		Buffer stagingBuffer = Buffer::createStagingBuffer(size);
		stagingBuffer.singleWrite(data, size, 0);
		stagingBuffer.copyTo(*this, size);
	}

	void Buffer::copyTo(Buffer& dest, VkDeviceSize size)
	{
		AGX_ASSERT_X((size == VK_WHOLE_SIZE) || (size <= m_bufferSize && size <= dest.m_bufferSize), 
			"Copy size exceeds source or destination buffer size");
		VulkanContext::device().copyBuffer(m_buffer, dest.m_buffer, size);
	}

	auto Buffer::computeAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) -> VkDeviceSize
	{
		if (minOffsetAlignment > 0)
			return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);

		return instanceSize;
	}

	void Buffer::destroy()
	{
		VulkanContext::destroy(m_buffer, m_allocation);
		m_buffer = VK_NULL_HANDLE;
		m_allocation = VK_NULL_HANDLE;
	}

	void Buffer::moveFrom(Buffer&& other)
	{
		m_buffer = std::exchange(other.m_buffer, VK_NULL_HANDLE);
		m_allocation = std::exchange(other.m_allocation, VK_NULL_HANDLE);
		m_bufferSize = std::exchange(other.m_bufferSize, 0);
		m_instanceSize = std::exchange(other.m_instanceSize, 0);
		m_alignmentSize = std::exchange(other.m_alignmentSize, 0);
		m_instanceCount = std::exchange(other.m_instanceCount, 0);
		m_usage = std::exchange(other.m_usage, 0);
		m_mapped = std::exchange(other.m_mapped, nullptr);
	}
}
