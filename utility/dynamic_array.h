#pragma once
#include "ranges.h"

namespace utility {
	template <typename element_type, typename size = u64>
	class dynamic_array {
	public:
		using iterator = element_type*;
		using const_iterator = const element_type*;

		using value_type = element_type;
		using size_type = size;

		dynamic_array() : m_data(nullptr), m_capacity(0), m_size(0) {}
		dynamic_array(const std::initializer_list<element_type>& values)
			: m_data(nullptr), m_capacity(0), m_size(0) {
			reserve(values.size());
			construct(values.begin(), values.end(), values.size());

			m_size = values.size();
		}
		dynamic_array(const dynamic_array& other)
			: m_data(nullptr), m_capacity(0), m_size(0) {
			reserve(other.get_size());
			construct(other.begin(), other.end(), other.get_size());

			m_size = other.get_size();
		}
		dynamic_array(dynamic_array&& other) noexcept {
			m_data = std::exchange(other.m_data, nullptr);
			m_capacity = std::exchange(other.m_capacity, 0);
			m_size = std::exchange(other.m_size, 0);
		}

		~dynamic_array() {
			clear();
			utility::free(m_data);
		}

		[[nodiscard]] auto operator=(const dynamic_array& other) -> dynamic_array& {
			if(this != &other) {
				clear();
				reserve(other.m_size);
				construct(other.begin(), other.end(), other.get_size());

				m_size = other.get_size();
			}

			return *this;
		}
		[[nodiscard]] auto operator=(dynamic_array&& other) noexcept -> dynamic_array& {
			m_data = std::exchange(other.m_data, nullptr);
			m_capacity = std::exchange(other.m_capacity, 0);
			m_size = std::exchange(other.m_size, 0);
			return *this;
		}

		[[nodiscard]] auto operator[](u64 index) -> value_type& {
			return m_data[index];
		}
		[[nodiscard]] auto operator[](u64 index) const -> const value_type& {
			return m_data[index];
		}

		void push_back(const element_type& value) {
			if(m_size >= m_capacity) {
				reserve(m_capacity > 0 ? m_capacity * 2 : 1);
			}

			if constexpr(std::is_trivial_v<element_type>) {
				m_data[m_size++] = value;
			}
			else {
				std::construct_at(&m_data[m_size++], value);
			}
		}

		template<typename... Args>
		auto emplace_back(Args&&... args) -> value_type& {
			if(m_size >= m_capacity) {
				reserve(m_capacity > 0 ? m_capacity * 2 : 1);
			}

			m_data[m_size++] = value_type(std::forward<Args>(args)...);
			return m_data[m_size - 1];
		}

		void pop_back() {
			if(empty()) {
				return;
			}

			--m_size;

			if constexpr(!std::is_trivial_v<element_type>) {
				std::destroy_at(&m_data[m_size]);
			}
		}

		void reserve(size_type new_capacity) {
			if(new_capacity <= m_capacity) {
				return;
			}

			element_type* new_data = static_cast<element_type*>(utility::malloc(new_capacity * sizeof(element_type)));

			if(new_data == nullptr) {
				throw std::bad_alloc();
			}

			if constexpr(std::is_trivial_v<element_type>) {
				utility::memcpy(new_data, m_data, m_size * sizeof(element_type));
			}
			else {
				construct_range(new_data, begin(), end());
				destruct_range(begin(), end());
			}

			utility::free(m_data);
			m_data = new_data;
			m_capacity = new_capacity;
		}

		void clear() {
			if constexpr(!std::is_trivial_v<element_type>) {
				destruct_range(begin(), end());
			}

			m_size = 0;
		}

		[[nodiscard]] auto empty() const -> bool {
			return m_size == 0;
		}

		[[nodiscard]] auto size() const -> u64 {
			return m_size;
		}

		[[nodiscard]] auto begin() -> iterator { return m_data; }
		[[nodiscard]] auto end() -> iterator { return m_data + m_size; }
		[[nodiscard]] auto begin() const -> const_iterator { return m_data; }
		[[nodiscard]] auto end() const -> const_iterator { return m_data + m_size; }
		[[nodiscard]] auto get_capacity() const -> size_type { return m_capacity; }
		[[nodiscard]] auto get_size() const -> size_type { return m_size; }
	protected:
		void construct(const_iterator begin, const_iterator end, size_type size) {
			if constexpr(std::is_trivial_v<element_type>) {
				utility::memcpy(m_data, begin, size);
			}
			else {
				construct_range(m_data, begin, end);
			}
		}
	protected:
		element_type* m_data;
		size_type m_capacity;
		size_type m_size;
	};
} // namespace utility