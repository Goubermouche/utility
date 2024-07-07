#pragma once

namespace utility {
	template<typename type>
	struct remove_pointer {
		using element_type = type;
	};

	template<typename type>
	struct remove_pointer<type*> {
		using element_type = type;
	};

	template<typename type>
	struct iterator_traits {
		using element_type = typename remove_pointer<type>::element_type;
	};

	template<typename type>
	constexpr bool is_trivial = __is_trivially_constructible(type) && __is_trivially_copyable(type);
} // namespace utility
