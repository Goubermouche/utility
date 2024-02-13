// memory_view utility header

#pragma once
#include "../allocators/allocator_base.h"
#include "../memory/contiguous_memory.h"

namespace utility {
	/**
	 * \brief Non-owning view into some contiguous block of memory.
	 * \tparam type Element type
	 * \tparam size_type Size type
	 */
	template<typename type, typename size_type = u64>
	class memory_view : public contiguous_memory<type, size_type> {
	public:
		using element_type = type;
		using base_type = contiguous_memory<type, size_type>;

		memory_view() = default;
		memory_view(void* data, size_type size) : base_type(static_cast<type*>(data), size) {}
		memory_view(type* data, size_type size) : base_type(data, size) {}

		/**
		 * \brief Allocates \b count of zero bytes of the element type and returns a view to it.
		 * \tparam allocator Allocator type to use (must derive from allocator_base)
		 * \param alloc Allocator to allocate the memory with
		 * \param count Number of elements to allocate
		 */
		template<typename allocator>
		requires is_allocator<allocator>
		memory_view(allocator& alloc, size_type count)
		: base_type(static_cast<type*>(alloc.allocate_zero(sizeof(type)* count)), count) {}

		[[nodiscard]] auto first() const -> type {
			return this->m_data[0];
		}

		[[nodiscard]] auto last() const -> type {
			return this->m_data[this->m_size - 1];
		}

		/**
		 * \brief Compares two slices of the same type.
		 * \param other Slice to compare with
		 * \return True if both slices match (their size is the same and the values it
		 * holds are the same). \b Does \b not \b check \b whether \b the \b data 
		 * \b pointers \b match.
		 */
		bool operator==(const memory_view& other) const {
			if(this->m_size != other.m_size) {
				return false;
			}

			for(size_type i = 0; i < this->m_size; ++i) {
				if (this->m_data[i] != other.m_data[i]) {
					return false;
				}
			}

			return true;
		}
	};
} // namespace utility
