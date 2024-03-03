// stack utility header

#pragma once
#include "../memory/memory_buffer.h"

// TODO: remove this

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
	};
} // namespace utility
