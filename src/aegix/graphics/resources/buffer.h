#pragma once

#include "graphics/bindless/descriptor_handle.h"
#include "graphics/globals.h"
#include "graphics/vulkan/volk_include.h"

#include <vk_mem_alloc.h>

namespace Aegix::Graphics
{
	/// @brief Encapsulates a vulkan buffer
	class Buffer
	{
	public:
		struct CreateInfo
		{
			VkDeviceSize instanceSize{ 0 };
			uint32_t instanceCount{ 1 };
			VkBufferUsageFlags usage{ 0 };
			VmaAllocationCreateFlags allocFlags{ 0 };
			VkDeviceSize minOffsetAlignment{ 0 };
		};

		/// @brief Factory methods for common buffer types
		/// @note Use multiple instances only if intended to be used with dynamic offsets (accessed by multiple descriptors)
		static auto uniformBuffer(VkDeviceSize size, uint32_t instanceCount = MAX_FRAMES_IN_FLIGHT) -> Buffer::CreateInfo;
		static auto storageBuffer(VkDeviceSize size, uint32_t instanceCount = 1) -> Buffer::CreateInfo;
		static auto vertexBuffer(VkDeviceSize size, uint32_t instanceCount = 1, VkBufferUsageFlags otherUsage = 0) -> Buffer::CreateInfo;
		static auto indexBuffer(VkDeviceSize size, uint32_t instanceCount = 1, VkBufferUsageFlags otherUsage = 0) -> Buffer::CreateInfo;
		static auto stagingBuffer(VkDeviceSize size, uint32_t instanceCount = 1, VkBufferUsageFlags otherUsage = 0) -> Buffer::CreateInfo;

		Buffer() = default;
		explicit Buffer(const CreateInfo& info);
		Buffer(const Buffer&) = delete;
		Buffer(Buffer&& other) noexcept;
		~Buffer();

		auto operator=(const Buffer&) -> Buffer& = delete;
		auto operator=(Buffer&& other) noexcept -> Buffer&;

		operator VkBuffer() const { return m_buffer; }

		[[nodiscard]] auto buffer() const -> VkBuffer { return m_buffer; }
		[[nodiscard]] auto bufferSize() const -> VkDeviceSize { return m_bufferSize; }
		[[nodiscard]] auto instanceSize() const -> VkDeviceSize { return m_instanceSize; }
		[[nodiscard]] auto alignmentSize() const -> VkDeviceSize { return m_alignmentSize; }
		[[nodiscard]] auto instanceCount() const -> uint32_t { return m_instanceCount; }
		[[nodiscard]] auto usage() const -> VkBufferUsageFlags { return m_usage; }
		[[nodiscard]] auto isMapped() const -> bool { return m_mapped != nullptr; }
		[[nodiscard]] auto descriptorBufferInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const -> VkDescriptorBufferInfo;
		[[nodiscard]] auto descriptorBufferInfoFor(uint32_t index) const -> VkDescriptorBufferInfo;

		/// @brief Map the buffer memory to allow writing to it 
		/// @note Consider using persistent mapped memory to avoid repeated map/unmap calls
		void map();

		/// @brief Unmap the buffer memory
		/// @note Consider using persistent mapped memory to avoid repeated map/unmap calls
		void unmap();

		// TODO: Rework these write functions to have:
		// Better naming (pretty inconsistent right now)
		// Unified behavior (some map/unmap internally, some don't)
		// Too many overloads (some with size/offset, some without)
		// Better utilize templates for type safety
		// Maybe offer casted pointer to data for easier writing?

		/// @brief Only writes data to the buffer
		/// @note Buffer MUST be mapped before calling
		void write(const void* data);
		void write(const void* data, VkDeviceSize size, VkDeviceSize offset);

		/// @brief Writes data of 'instanceSize' to the buffer at an offset of 'index * alignmentSize'
		/// @note Buffer MUST be mapped before calling
		void writeToIndex(const void* data, uint32_t index);

		/// @brief Maps, writes data, then unmaps the buffer
		void singleWrite(const void* data);
		void singleWrite(const void* data, VkDeviceSize size, VkDeviceSize offset);

		/// @brief Flushes the buffer memory to make it visible to the device 
		/// @note Only required for non-coherent memory, 'write' functions already flush
		void flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		/// @brief Flush the memory range at 'index * alignmentSize'
		void flushIndex(uint32_t index);

		/// @brief Uploads data to the buffer using a staging buffer (Used for device local memory)
		void upload(const void* data, VkDeviceSize size);

		/// @brief Copy the buffer to another buffer
		void copyTo(Buffer& dest, VkDeviceSize size);
		void copyTo(VkCommandBuffer cmd, Buffer& dest, uint32_t srcIndex, uint32_t destIndex) const;

		template<typename T>
		void upload(const std::vector<T>& data)
		{
			AGX_ASSERT_X(!data.empty(), "Data vector is empty");
			AGX_ASSERT_X(sizeof(T) * data.size() <= m_bufferSize, "Data size exceeds buffer size");
			upload(data.data(), sizeof(T) * data.size());
		}

		template<typename T = void>
		auto data(uint32_t index = 0) -> T*
		{
			AGX_ASSERT_X(m_mapped, "Called mappedAs on buffer before map");
			AGX_ASSERT_X(sizeof(T) <= m_instanceSize, "Mapped type size exceeds instance size");
			AGX_ASSERT_X(index < m_instanceCount, "Mapped index exceeds instance count");
			return reinterpret_cast<T*>(static_cast<uint8_t*>(m_mapped) + index * m_alignmentSize);
		}

	private:
		static auto computeAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) -> VkDeviceSize;

		void destroy();
		void moveFrom(Buffer&& other);

		VkBuffer m_buffer{ VK_NULL_HANDLE };
		VmaAllocation m_allocation{ VK_NULL_HANDLE };
		VkDeviceSize m_bufferSize{ 0 };
		VkDeviceSize m_alignmentSize{ 0 };
		VkDeviceSize m_instanceSize{ 0 };
		uint32_t m_instanceCount{ 0 };
		VkBufferUsageFlags m_usage{ 0 };
		void* m_mapped{ nullptr};
	};
}
