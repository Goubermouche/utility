// bit set utility header

#pragma once
#include "../allocators/allocator_base.h"

namespace utility {
	class bit_set {
		class bit_proxy {
		public:
			bit_proxy(u64& word, u64 bit) : m_word(word), m_mask(1ULL << bit) {}

			auto operator=(bool value) -> bit_proxy& {
				if (value) {
					m_word |= m_mask;
				}
				else {
					m_word &= ~m_mask;
				}

				return *this;
			}

			operator bool() const {
				return (m_word & m_mask) != 0;
			}
		private:
			u64& m_word;
			u64 m_mask;
		};
	public:
		template<typename allocator>
		requires is_allocator<allocator>
		bit_set(u64 size, allocator& alloc)
		: m_bits(static_cast<u64*>(alloc.allocate_zero(sizeof(u64)* (size + 63) / 64))), m_bit_size(size) {}

		auto operator[](u64 index) const -> bool {
			const u64 word_index = index / 64;
			const u64 bit_index = index % 64;

			return (m_bits[word_index] & (static_cast<u64>(1) << bit_index)) != 0;
		}

		auto operator[](u64 index) -> bit_proxy {
			return { m_bits[index / 64], index % 64 };
		}

		void print() const {
			for (u64 i = 0; i < m_bit_size; ++i) {
				std::cout << (*this)[i];
			}

			std::cout << '\n';
		}
	private:
		u64* m_bits;
		u64 m_bit_size;
	};
} // namespace utility
