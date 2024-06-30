#pragma once
#include "../ranges.h"
#include "../assert.h"

namespace utility {
	template <typename value_type, typename size_type = u64>
	class dynamic_array {
	public:
		using iterator = value_type*;
		using const_iterator = const value_type*;

		using element_type = value_type;

		dynamic_array() : m_data(nullptr), m_capacity(0), m_size(0) {}
	
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

		auto operator=(const dynamic_array& other) -> dynamic_array& {
			if(this != &other) {
				clear();
				reserve(other.m_size);
				construct(other.begin(), other.end(), other.get_size());

				m_size = other.get_size();
			}

			return *this;
		}
		auto operator=(dynamic_array&& other) noexcept -> dynamic_array& {
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

		void push_back(const value_type& value) {
			if(m_size >= m_capacity) {
				reserve(m_capacity > 0 ? m_capacity * 2 : 1);
			}

			if constexpr(std::is_trivial_v<value_type>) {
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

			if constexpr(!std::is_trivial_v<value_type>) {
				std::destroy_at(&m_data[m_size]);
			}
		}

		void reserve(size_type new_capacity) {
			if(new_capacity <= m_capacity) {
				return;
			}

			value_type* new_data = static_cast<value_type*>(utility::malloc(new_capacity * sizeof(value_type)));

			if(new_data == nullptr) {
				throw std::bad_alloc();
			}

			if constexpr(std::is_trivial_v<value_type>) {
				utility::memcpy(new_data, m_data, m_size * sizeof(value_type));
			}
			else {
				construct_range(new_data, begin(), end());
				destruct_range(begin(), end());
			}

			utility::free(m_data);
			m_data = new_data;
			m_capacity = new_capacity;
		}

		template<typename iterator_type>
		void insert(iterator pos, iterator_type first, iterator_type last) {
			if(first == last) {
				return;
			}

			size_type num_elements_to_insert = distance(first, last);
			size_type index = distance(begin(), pos);

			// ensure there is enough space for the new elements
			if(m_size + num_elements_to_insert > m_capacity) {
				reserve(m_size + num_elements_to_insert);
			}

			// move existing elements to make space for the new elements
			if constexpr(std::is_trivial_v<value_type>) {
				utility::memmove(
					m_data + index + num_elements_to_insert, 
					m_data + index, 
					(m_size - index) * sizeof(value_type)
				);
			}
			else {
				// move construct elements from end to start to prevent overwriting
				for(size_type i = m_size; i > index; --i) {
					new (m_data + i + num_elements_to_insert - 1) value_type(std::move(m_data[i - 1]));
					m_data[i - 1].~value_type(); // destroy the old object after moving
				}
			}

			// copy new elements into the space created
			auto insert_pos = m_data + index;

			for(auto it = first; it != last; ++it, ++insert_pos) {
				new (insert_pos) value_type(*it); // copy construct new elements
			}

			m_size += num_elements_to_insert;
		}

		void clear() {
			if constexpr(!std::is_trivial_v<value_type>) {
				destruct_range(begin(), end());
			}

			m_size = 0;
		}

		[[nodiscard]] auto empty() const -> bool {
			return m_size == 0;
		}

		[[nodiscard]] auto get_data() const -> value_type* {
			return m_data;
		}

		[[nodiscard]] auto begin() -> iterator { return m_data; }
		[[nodiscard]] auto end() -> iterator { return m_data + m_size; }
		[[nodiscard]] auto begin() const -> const_iterator { return m_data; }
		[[nodiscard]] auto end() const -> const_iterator { return m_data + m_size; }
		[[nodiscard]] auto get_capacity() const -> size_type { return m_capacity; }
		[[nodiscard]] auto get_size() const -> size_type { return m_size; }
	protected:
		void construct(const_iterator begin, const_iterator end, size_type size) {
			if constexpr(std::is_trivial_v<value_type>) {
				utility::memcpy(m_data, begin, size * sizeof(element_type));
			}
			else {
				construct_range(m_data, begin, end);
			}
		}
	protected:
		value_type* m_data;
		size_type m_capacity;
		size_type m_size;
	};
} // namespace utility
