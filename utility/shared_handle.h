// handle utility header

#pragma once
#include "allocators/allocator_base.h"
#include "handle.h"

namespace utility::types {
	/**
	 * \brief Higher-level handle. Used as an abstraction for objects allocated via an allocator. Once
	 * the lifetime of the shared handle container and all of it's references reaches its end the contained
	 * object's destructor gets called. 
	 * \tparam type 
	 * \tparam count_type 
	 */
	template<typename type, typename count_type = u64>
	class shared_handle {
	public:
		template<typename other_type>
		shared_handle(const shared_handle<other_type>& other)
		: m_ptr(other.get()), m_count(other.get_count()) {}

		template<typename allocator>
		requires is_allocator<allocator>
		shared_handle(allocator& alloc)
		: m_ptr(alloc.emplace<type>()), m_count(alloc.emplace<count_type>(1)) {}

		template<typename other_type>
		auto operator=(const shared_handle<other_type>& other) -> shared_handle& {
			if (this != &other) {
				release();
				m_ptr = other.m_ptr;
				m_count = other.m_count;
				(*m_count)++;
			}

			return *this;
		}

		auto operator->() const -> type* {
			return m_ptr;
		}

		~shared_handle() {
			release();
		}

		auto get() const -> type*{
			return m_ptr;
		}

		auto get_use_count() const -> count_type {
			return (m_count ? *m_count : 0);
		}

		auto get_count() const -> count_type* {
			return m_count;
		}
	private:
		void release() const {
			if (m_count && --(*m_count) == 0) {
				m_ptr->~type();
				m_count->~count_type();
			}
		}
	private:
		type* m_ptr = nullptr;
		count_type* m_count;
	};
} // namespace utility
