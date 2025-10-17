#pragma once

#include "graphics/globals.h"
#include "graphics/vulkan/volk_include.h"

#include <vk_mem_alloc.h>

namespace Aegix::Graphics
{
	/// @brief Encapsulates a vulkan buffer
	class Buffer
	{
	public:
		/// @brief Factory methods for common buffer types
		/// @note Use multiple instances only if intended to be used with dynamic offsets (accessed by multiple descriptors)
		static auto createUniformBuffer(VkDeviceSize size, uint32_t instanceCount = MAX_FRAMES_IN_FLIGHT) -> Buffer;
		static auto createStorageBuffer(VkDeviceSize size, uint32_t instanceCount = 1) -> Buffer;
		static auto createVertexBuffer(VkDeviceSize size, uint32_t instanceCount = 1) -> Buffer;
		static auto createIndexBuffer(VkDeviceSize size, uint32_t instanceCount = 1) -> Buffer;
		static auto createStagingBuffer(VkDeviceSize size) -> Buffer;

		Buffer(VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags bufferUsage,
			VmaAllocationCreateFlags allocFlags = 0, VkDeviceSize minOffsetAlignment = 0);
		Buffer(const Buffer&) = delete;
		Buffer(Buffer&& other) noexcept;
		~Buffer();

		auto operator=(const Buffer&) -> Buffer& = delete;
		auto operator=(Buffer&& other) noexcept -> Buffer&;

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

		/// @brief Uploads data to the buffer using a staging buffer (Used for device local memory)
		void upload(const void* data, VkDeviceSize size);

		/// @brief Copy the buffer to another buffer
		void copyTo(Buffer& dest, VkDeviceSize size);

	private:
		static auto computeAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) -> VkDeviceSize;

		void destroy();

		VkBuffer m_buffer = VK_NULL_HANDLE;
		VmaAllocation m_allocation = VK_NULL_HANDLE;
		VkDeviceSize m_bufferSize = 0;
		VkDeviceSize m_instanceSize = 0;
		VkDeviceSize m_alignmentSize = 0;
		uint32_t m_instanceCount = 0;
		VkBufferUsageFlags m_usage = 0;
		void* m_mapped = nullptr;
	};
}
