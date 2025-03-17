#pragma once

#include "graphics/device.h"

namespace Aegix::Graphics
{
	/// @brief Encapsulates a vulkan buffer
	class Buffer
	{
	public:
		static auto createUniformBuffer(VulkanDevice& device, VkDeviceSize size, uint32_t instanceCount = MAX_FRAMES_IN_FLIGHT) -> Buffer;
		static auto createStagingBuffer(VulkanDevice& device, VkDeviceSize size) -> Buffer;

		Buffer(VulkanDevice& device, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags bufferUsage,
			VmaAllocationCreateFlags allocFlags = 0, VkDeviceSize minOffsetAlignment = 0);
		Buffer(const Buffer&) = delete;
		~Buffer();

		Buffer& operator=(const Buffer&) = delete;
		
		operator VkBuffer() const { return m_buffer; }

		[[nodiscard]] auto buffer() const -> VkBuffer { return m_buffer; }
		[[nodiscard]] auto bufferSize() const -> VkDeviceSize { return m_bufferSize; }
		[[nodiscard]] auto instanceSize() const -> VkDeviceSize { return m_instanceSize; }
		[[nodiscard]] auto alignmentSize() const -> VkDeviceSize { return m_instanceSize; }
		[[nodiscard]] auto instanceCount() const -> uint32_t { return m_instanceCount; }
		[[nodiscard]] auto usage() const -> VkBufferUsageFlags { return m_usage; }

		[[nodiscard]] auto descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const -> VkDescriptorBufferInfo;
		[[nodiscard]] auto descriptorInfoForIndex(int index) const -> VkDescriptorBufferInfo;

		/// @brief Map the buffer memory to allow writing to it
		void map();

		/// @brief Unmap the buffer memory
		void unmap();

		/// @brief Only writes data to the buffer
		/// @note Buffer MUST be mapped before calling
		void write(const void* data);
		void write(const void* data, VkDeviceSize size, VkDeviceSize offset);

		/// @brief Writes data of 'instanceSize' to the buffer at an offset of 'index * alignmentSize'
		/// @note Buffer MUST be mapped before calling
		void writeToIndex(const void* data, int index);
		void writeToAll(const void* data);

		/// @brief Maps, writes data, then unmaps the buffer
		void singleWrite(const void* data);
		void singleWrite(const void* data, VkDeviceSize size, VkDeviceSize offset);

		/// @brief Flushes the buffer memory to make it visible to the device 
		/// @note Only required for non-coherent memory, 'write' functions already flush
		void flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		/// @brief Flush the memory range at 'index * alignmentSize'
		void flushIndex(int index);

	private:
		static auto computeAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) -> VkDeviceSize;

		VulkanDevice& m_device;

		VkBuffer m_buffer = VK_NULL_HANDLE;
		VmaAllocation m_allocation = VK_NULL_HANDLE;
		VkDeviceSize m_bufferSize;
		VkDeviceSize m_instanceSize;
		VkDeviceSize m_alignmentSize;
		uint32_t m_instanceCount;
		VkBufferUsageFlags m_usage;
		void* m_mapped = nullptr;
	};
}
