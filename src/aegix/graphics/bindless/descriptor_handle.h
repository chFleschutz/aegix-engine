#pragma once

namespace Aegix::Graphics
{
	/// @note This has to match the bindless::Handle struct on the slang side
	/// @see shaders/modules/bindless.slang
	class DescriptorHandle
	{
		friend class DescriptorHandleCache;

	public:
		enum class Type : uint8_t
		{
			SampledImage = 0,
			StorageImage = 1,
			StorageBuffer = 2,
			UniformBuffer = 3,
		};

		static constexpr uint32_t INVALID_HANDLE = std::numeric_limits<uint32_t>::max();

		static constexpr uint32_t INDEX_BITS = 24;
		static constexpr uint32_t VERSION_BITS = 6;
		static constexpr uint32_t TYPE_BITS = 2;

		static constexpr uint32_t INDEX_MASK = (1 << INDEX_BITS) - 1;
		static constexpr uint32_t VERSION_MASK = (1 << VERSION_BITS) - 1;
		static constexpr uint32_t TYPE_MASK = (1 << TYPE_BITS) - 1;

		DescriptorHandle() = default;
		~DescriptorHandle() = default;

		[[nodiscard]] auto index() const -> uint32_t { return m_handle & INDEX_MASK; }
		[[nodiscard]] auto version() const -> uint32_t { return (m_handle >> INDEX_BITS) & VERSION_MASK; }
		[[nodiscard]] auto type() const -> Type { return static_cast<Type>((m_handle >> (INDEX_BITS + VERSION_BITS)) & TYPE_MASK); }
		[[nodiscard]] auto isValid() const -> bool { return m_handle != INVALID_HANDLE; }

		void invalidate() { m_handle = INVALID_HANDLE; }

	private:
		DescriptorHandle(uint32_t index, Type type);

		auto pack(uint32_t index, uint32_t version, Type type) -> uint32_t;
		void recycle(Type type);

		// Packed as: | 2 bits type | 6 bits version | 24 bits index |
		uint32_t m_handle{ INVALID_HANDLE };
	};
}