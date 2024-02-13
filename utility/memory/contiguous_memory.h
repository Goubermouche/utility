// contiguous_memory utility header

#pragma once
#include "memory.h"

namespace utility {
	template<typename type, typename size_type = u64>
	class contiguous_memory {
	public:
		// iterators
		using iterator = type*;
		using const_iterator = const type*;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		contiguous_memory() : m_data(nullptr), m_size(0) {}
		contiguous_memory(type* data, size_type size) : m_data(data), m_size(size) {}

		[[nodiscard]] auto get_data() const -> type* {
			return m_data;
		}

		[[nodiscard]] auto get_size() const -> size_type {
			return m_size;
		}

		[[nodiscard]] constexpr auto is_empty() const -> bool {
			return this->m_size == 0;
		}

		[[nodiscard]] auto operator[](u64 index) -> type& {
			return this->m_data[index];
		}

		[[nodiscard]] auto operator[](u64 index) const -> const type& {
			return this->m_data[index];
		}

		[[nodiscard]] auto begin() -> iterator {
			return this->m_data;
		}

		[[nodiscard]] auto begin() const -> const_iterator {
			return this->m_data;
		}

		[[nodiscard]] auto rbegin() -> reverse_iterator {
			return reverse_iterator(end());
		}

		[[nodiscard]] auto rbegin() const -> const_reverse_iterator {
			return const_reverse_iterator(end());
		}

		[[nodiscard]] auto end() const -> const_iterator {
			return this->m_data + this->m_size;
		}

		[[nodiscard]] auto end() -> iterator {
			return this->m_data + this->m_size;
		}

		[[nodiscard]] auto rend() -> reverse_iterator {
			return reverse_iterator(begin());
		}

		[[nodiscard]] auto rend() const -> const_reverse_iterator {
			return const_reverse_iterator(begin());
		}
	protected:
		type* m_data;
		size_type m_size;
	};

	template<typename type, typename size_type = u64>
	void copy(contiguous_memory<type, size_type>& destination, const std::vector<type>& source) {
		ASSERT(destination.get_size() >= source.size(), "incompatible sizes");
		std::memcpy(destination.get_data(), source.data(), source.size() * sizeof(type));
	}

	template<typename type, typename size_type = u64>
	void copy(contiguous_memory<type, size_type>& destination, u64 begin_offset, const std::vector<type>& source) {
		ASSERT(destination.get_size() + begin_offset >= source.size(), "incompatible sizes");
		std::memcpy(destination.get_data() + begin_offset, source.data(), source.size() * sizeof(type));
	}

	template<typename type, typename size_type = u64>
	void copy(contiguous_memory<type, size_type>& destination, const contiguous_memory<type, size_type>& source) {
		ASSERT(destination.get_size() >= source.get_size(), "incompatible sizes");
		std::memcpy(destination.get_data(), source.get_data(), source.get_size() * sizeof(type));
	}
} // namespace utility
