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
	constexpr bool is_trivial_v = __is_trivially_constructible(type) && __is_trivially_copyable(type);

	template <class type>
	struct remove_reference {
		using element_type = type;
		using _Const_thru_ref_type = const type;
	};

	template <class type>
	struct remove_reference<type&> {
		using element_type = type;
		using _Const_thru_ref_type = const type&;
	};

	template <class type>
	struct remove_reference<type&&> {
		using element_type = type;
		using _Const_thru_ref_type = const type&&;
	};

	template <class type>
	using remove_reference_t = typename remove_reference<type>::element_type;

	template <class>
	constexpr bool is_lvalue_reference_v = false;

	template <class _Ty>
	constexpr bool is_lvalue_reference_v<_Ty&> = true;

	template <typename type, typename other = type>
	constexpr auto exchange(type& value, other&& new_value) noexcept -> type {
		type old = static_cast<type&&>(value);
#pragma warning(push)
#pragma warning(disable: 4244)
		value = static_cast<other&&>(new_value);
#pragma warning(pop)
		return old;
	}

	template<typename type>
	constexpr auto forward(remove_reference_t<type>& arg) noexcept -> type&& {
		return static_cast<type&&>(arg);
	}

	template<typename type>
	constexpr auto forward(remove_reference_t<type>&& arg) noexcept -> type&& {
		static_assert(!is_lvalue_reference_v<type>, "bad forward call");
		return static_cast<type&&>(arg);
	}

	template<typename type, typename... types>
	constexpr auto construct_at(type* const location, types&&... args) noexcept(
		noexcept(::new(static_cast<void*>(location)) type(forward<types>(args)...))
	) -> type* {
		return ::new(static_cast<void*>(location)) type(forward<types>(args)...);
	}

	template<typename type>
	void destroy_at(type* const location) noexcept {
		location->~type();
	}

	template<typename type>
	constexpr auto move(type&& value) noexcept -> remove_reference_t<type>&& {
		return static_cast<remove_reference_t<type>&&>(value);
	}

	template<typename input, typename output>
	void move(input first, input last, output result) {
		while(first != last) {
			*result = move(*first);
			++first;
			++result;
		}
	}

	template<typename type>
	void swap(type& left, type& right) noexcept {
		type temp = move(left);
		left = move(right);
		right = move(temp);
	}
} // namespace utility
