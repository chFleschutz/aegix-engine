#pragma once

namespace Aegix::Graphics
{
	class DescriptorHandle
	{
		friend class DescriptorHandleCache;

	public:
		enum class Type : uint8_t
		{
			Buffer = 0,
			Texture = 1,
			RWTexture = 2
		};

		enum class Access : uint8_t
		{
			ReadOnly = 0,
			ReadWrite = 1
		};

		static constexpr uint32_t INDEX_BITS = 23;
		static constexpr uint32_t TYPE_BITS = 2;
		static constexpr uint32_t ACCESS_BITS = 1;
		static constexpr uint32_t VERSION_BITS = 6;

		static constexpr uint32_t INDEX_MASK = (1u << INDEX_BITS) - 1;
		static constexpr uint32_t TYPE_MASK = (1u << TYPE_BITS) - 1;
		static constexpr uint32_t ACCESS_MASK = (1u << ACCESS_BITS) - 1;
		static constexpr uint32_t VERSION_MASK = (1u << VERSION_BITS) - 1;

		static constexpr uint32_t INVALID_HANDLE = std::numeric_limits<uint32_t>::max();

		DescriptorHandle() = default;
		~DescriptorHandle() = default;

		[[nodiscard]] auto index() const -> uint32_t { return m_handle & INDEX_MASK; }
		[[nodiscard]] auto type() const -> Type { return static_cast<Type>((m_handle >> INDEX_BITS) & TYPE_MASK); }
		[[nodiscard]] auto access() const -> Access { return static_cast<Access>((m_handle >> (INDEX_BITS + TYPE_BITS)) & ACCESS_MASK); }
		[[nodiscard]] auto version() const -> uint32_t { return (m_handle >> (INDEX_BITS + TYPE_BITS + ACCESS_BITS)) & VERSION_MASK; }
		[[nodiscard]] auto isValid() const -> bool { return m_handle != INVALID_HANDLE; }

		void invalidate() { m_handle = INVALID_HANDLE; }

	private:
		DescriptorHandle(uint32_t index, Type type, Access access);

		auto createHandle(uint32_t index, Type type, Access access, uint32_t version) -> uint32_t;
		void recycle(Type type, Access access);

		// Packed as: | 6 bits version | 1 bit access | 2 bits type | 23 bits index |
		uint32_t m_handle{ INVALID_HANDLE };
	};
}