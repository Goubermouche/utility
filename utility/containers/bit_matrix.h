// bit matrix utility header

#pragma once
#include "../containers/bit_set.h"
#include "../memory/memory_view.h"

namespace utility {
	class bit_matrix {
	public:
		template<typename allocator>
		requires is_allocator<allocator>
		bit_matrix(u64 width, u64 height, allocator& alloc)
		: m_rows(alloc, height) {
			for(u64 i = 0; i < width; ++i) {
				m_rows[i] = bit_set(width, alloc);
			}
		}

		void print() const {
			for (const bit_set& set : m_rows) {
				set.print();
			}
		}

		auto operator[](u64 index) const -> const bit_set& {
			return m_rows[index];
		}

		auto operator[](u64 index) -> bit_set& {
			return m_rows[index];
		}
	private:
		memory_view<bit_set> m_rows;
	};
} // namespace utility
