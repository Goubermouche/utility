// allocator base utility header

#pragma once
#include "../types.h"

namespace utility {
	/**
	 * \brief Base for all utility allocators.
	 */
	class allocator_base {};

	template<typename type>
	inline constexpr bool is_allocator = std::is_base_of_v<allocator_base, type>;
}
