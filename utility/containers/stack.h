// stack utility header

#pragma once
#include "../memory/memory_buffer.h"

namespace utility {
	template<typename type>
	class stack : public memory_buffer<type> {
	public:
		stack()
			: memory_buffer<type>() {}

		stack(const std::initializer_list<type>& initializer_list)
			: memory_buffer<type>(initializer_list) {}

		stack(const memory_buffer<type>& container)
			: memory_buffer<type>(container) {}

		[[nodiscard]] constexpr auto pop_back() -> type& {
			return this->m_data[--this->m_size];
		}

		constexpr void pop() {
			--this->m_size;
		}
	private:

	};
} // namespace utility
