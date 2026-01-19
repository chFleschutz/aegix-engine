#pragma once 

#include "graphics/bindless/descriptor_handle.h"
#include "graphics/resources/buffer.h"

namespace Aegis::Graphics
{
	/// @brief Encapsulates a buffer with a single descriptor handle
	class BindlessBuffer
	{
	public:
		BindlessBuffer() = default;
		explicit BindlessBuffer(const Buffer::CreateInfo& bufferInfo);
		BindlessBuffer(const BindlessBuffer&) = delete;
		BindlessBuffer(BindlessBuffer&& other) noexcept;
		~BindlessBuffer();

		auto operator=(const BindlessBuffer&) -> BindlessBuffer& = delete;
		auto operator=(BindlessBuffer&& other) noexcept -> BindlessBuffer&;

		operator Buffer& () { return m_buffer; }
		operator const Buffer& () const { return m_buffer; }

		[[nodiscard]] auto buffer() -> Buffer& { return m_buffer; }
		[[nodiscard]] auto buffer() const -> const Buffer& { return m_buffer; }
		[[nodiscard]] auto handle() const -> DescriptorHandle { return m_handle; }

	private:
		Buffer m_buffer;
		DescriptorHandle m_handle;
	};


	/// @brief Encapsulates a buffer with multiple descriptor handles (one per buffer instance)
	class BindlessMultiBuffer
	{
	public:
		BindlessMultiBuffer() = default;
		explicit BindlessMultiBuffer(const Buffer::CreateInfo& bufferInfo);
		BindlessMultiBuffer(const BindlessMultiBuffer&) = delete;
		BindlessMultiBuffer(BindlessMultiBuffer&& other) noexcept;
		~BindlessMultiBuffer();

		auto operator=(const BindlessMultiBuffer&) -> BindlessMultiBuffer& = delete;
		auto operator=(BindlessMultiBuffer&& other) noexcept -> BindlessMultiBuffer&;

		[[nodiscard]] auto buffer() -> Buffer& { return m_buffer; }
		[[nodiscard]] auto buffer() const -> const Buffer& { return m_buffer; }
		[[nodiscard]] auto handle(size_t index = 0) const -> DescriptorHandle
		{
			AGX_ASSERT_X(index < m_handles.size(), "BindlessMultiBuffer handle index out of bounds");
			return m_handles[index];
		}

	private:
		Buffer m_buffer;
		std::vector<DescriptorHandle> m_handles;
	};


	/// @brief Encapsulates a buffer with multiple descriptor handles (one per frame in flight)
	class BindlessFrameBuffer
	{
	public:
		BindlessFrameBuffer() = default;
		explicit BindlessFrameBuffer(const Buffer::CreateInfo& bufferInfo);
		BindlessFrameBuffer(const BindlessFrameBuffer&) = delete;
		BindlessFrameBuffer(BindlessFrameBuffer&&) = default;
		~BindlessFrameBuffer();
		
		[[nodiscard]] auto buffer() -> Buffer& { return m_buffer; }
		[[nodiscard]] auto buffer() const -> const Buffer& { return m_buffer; }
		[[nodiscard]] auto handle(size_t index) const -> DescriptorHandle;

		void write(const void* data, VkDeviceSize size, VkDeviceSize offset, size_t index)
		{
			AGX_ASSERT_X(index < MAX_FRAMES_IN_FLIGHT, "BindlessFrameBuffer write index out of bounds");
			m_buffer.write(data, size, index * m_buffer.alignmentSize() + offset);
		}

	private:
		Buffer m_buffer;
		std::array<DescriptorHandle, MAX_FRAMES_IN_FLIGHT> m_handles;
	};
}
