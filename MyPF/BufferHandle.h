#pragma once
#include <cstdint>
#include <limits>

namespace My
{
	class BufferHandle
	{
	public:
		BufferHandle()
			: m_index{ m_invalidIndex }
		{
		}

		explicit BufferHandle(uint32_t index)
			: m_index(index)
		{
		}

		uint32_t GetIndex() const { return m_index; }

		bool IsValid() const
		{
			return m_index != m_invalidIndex;
		}

	private:
		// 무효 인덱스 상수
		static const uint32_t m_invalidIndex = (std::numeric_limits<uint32_t>::max)();
		uint32_t m_index;
	};
}