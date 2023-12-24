#pragma once

#include "graphics/device.h"

namespace VEGraphics
{
	/// @brief Encapsulates a vulkan buffer
	class Buffer
	{
	public:
		Buffer(
			VulkanDevice& device,
			VkDeviceSize instanceSize,
			uint32_t instanceCount,
			VkBufferUsageFlags usageFlags,
			VkMemoryPropertyFlags memoryPropertyFlags,
			VkDeviceSize minOffsetAlignment = 1);
		~Buffer();

		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;

		/// @brief Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
		/// @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete buffer range.
		/// @param offset (Optional) Byte offset from beginning
		/// @return VkResult of the buffer mapping call
		VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		/// @brief Unmap a mapped memory range
		/// @note Does not return a result as vkUnmapMemory can't fail
		void unmap();

		/// @brief Copies the specified data to the mapped buffer. Default value writes whole buffer range
		/// @param data Pointer to the data to copy
		/// @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer range.
		/// @param offset (Optional) Byte offset from beginning of mapped region
		void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		/// @brief Flush a memory range of the buffer to make it visible to the device
		/// @note Only required for non-coherent memory
		/// @param size (Optional) Size of the memory range to flush (Pass VK_WHOLE_SIZE to flush the complete buffer range)
		/// @param offset (Optional) Byte offset from beginning
		/// @return VkResult of the flush call
		VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		/// @brief Invalidate a memory range of the buffer to make it visible to the host
		/// @note Only required for non-coherent memory
		/// @param size (Optional) Size of the memory range to invalidate (Pass VK_WHOLE_SIZE to invalidate the complete buffer range)
		/// @param offset (Optional) Byte offset from beginning
		/// @return VkResult of the invalidate call
		VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		/// @brief Create a buffer info descriptor
		/// @param size (Optional) Size of the memory range of the descriptor
		/// @param offset (Optional) Byte offset from beginning
		/// @return VkDescriptorBufferInfo of specified offset and range
		VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		/// @brief Copies "instanceSize" bytes of data to the mapped buffer at an offset of index alignmentSize
		/// @param data Pointer to the data to copy
		/// @param index Used in offset calculation
		void writeToIndex(void* data, int index);
		/// @brief Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
		/// @param index index Used in offset calculation
		/// @return VkResult of the flush call
		VkResult flushIndex(int index);
		/// @brief Create a buffer info descriptor
		/// @param index Specifies the region given by index * alignmentSize
		/// @return VkDescriptorBufferInfo for instance at index
		VkDescriptorBufferInfo descriptorInfoForIndex(int index);
		/// @brief Invalidate a memory range of the buffer to make it visible to the host
		/// @note Only required for non-coherent memory
		/// @param index Specifies the region to invalidate: index * alignmentSize
		/// @return VkResult of the invalidate call
		VkResult invalidateIndex(int index);

		VkBuffer buffer() const { return m_buffer; }
		void* mappedMemory() const { return m_mapped; }
		uint32_t instanceCount() const { return m_instanceCount; }
		VkDeviceSize instanceSize() const { return m_instanceSize; }
		VkDeviceSize alignmentSize() const { return m_instanceSize; }
		VkBufferUsageFlags usageFlags() const { return m_usageFlags; }
		VkMemoryPropertyFlags memoryPropertyFlags() const { return m_memoryPropertyFlags; }
		VkDeviceSize bufferSize() const { return m_bufferSize; }

	private:
		/// @brief Returns the minimum instance size required to be compatible with devices minOffsetAlignment
		/// @param instanceSize The size of an instance
		/// @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg minUniformBufferOffsetAlignment)
		/// @return VkResult of the buffer mapping call
		static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

		VulkanDevice& m_device;
		void* m_mapped = nullptr;
		VkBuffer m_buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_memory = VK_NULL_HANDLE;

		VkDeviceSize m_bufferSize;
		uint32_t m_instanceCount;
		VkDeviceSize m_instanceSize;
		VkDeviceSize m_alignmentSize;
		VkBufferUsageFlags m_usageFlags;
		VkMemoryPropertyFlags m_memoryPropertyFlags;
	};

}  // namespace vre