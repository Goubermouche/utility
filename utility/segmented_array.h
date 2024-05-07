#pragma once
#include "ranges.h"

namespace utility {
	template<typename type>
	class segmented_array {
		struct segment {
			~segment() {
				if constexpr(!std::is_trivial_v<type>) {
					destruct_range(data, data + size);
				}

				utility::free(data);
			}

			type* data;
			u64 size;
			u64 capacity;
			segment* next;
		};

		struct iterator {
		public:
			iterator(segment* segment, u64 index) : m_segment(segment), m_index(index) {}

			auto operator++() -> iterator& {
				if(m_index + 1 >= m_segment->size && m_segment->next) {
					m_segment = m_segment->next;
					m_index = 0;
					return *this;
				}

				m_index++;
				return *this;
			}

			auto operator+(u64 offset) -> iterator& {
				for(u64 i = 0; i < offset; ++i) {
					this->operator++();
				}

				return *this;
			}

			auto operator*() -> type& {
				return m_segment->data[m_index];
			}

			auto operator==(const iterator& other) const -> bool {
				return m_segment == other.m_segment && m_index == other.m_index;
			}

			auto operator!=(const iterator& other) const -> bool {
				return !(*this == other);
			}

			segment* m_segment;
			u64 m_index;
		};
	public:
		segmented_array() : m_first_segment(nullptr), m_current_segment(nullptr), m_segment_capacity(0), m_size(0) {}
		segmented_array(u64 segment_size) : m_first_segment(nullptr), m_current_segment(nullptr), m_segment_capacity(segment_size), m_size(0) {
			m_first_segment = allocate_segment(m_segment_capacity);
			m_current_segment = m_first_segment;
		}

		segmented_array(const segmented_array& other) : m_segment_capacity(other.m_segment_capacity), m_size(other.m_size) {
			if(other.m_first_segment == nullptr) {
				return;
			}

			m_first_segment = allocate_segment();
			m_current_segment = m_first_segment;
			segment* current = other.m_first_segment;

			while(current != nullptr) {
				m_current_segment->size = current->size;

				if constexpr(std::is_trivial_v<type>) {
					utility::memcpy(m_current_segment->data, current->data, sizeof(type) * current->size);
				}
				else {
					construct_range(m_current_segment->data, current->data, current->data + current->size);
				}

				if(current->next) {
					m_current_segment->next = allocate_segment();
					m_current_segment = m_current_segment->next;
				}

				current = current->next;
			}
		}

		~segmented_array() {
			deallocate();
		}

		void push_back(const type& value) {
			if(m_current_segment->size >= m_current_segment->capacity) {
				segment* next = allocate_segment(m_segment_capacity);
				m_current_segment->next = next;
				m_current_segment = next;
			}

			if constexpr(std::is_trivial_v<type>) {
				m_current_segment->data[m_current_segment->size++] = value;
			}
			else {
				std::construct_at(&m_current_segment->data[m_current_segment->size++], value);
			}

			m_size++;
		}

		template<typename iterator_type>
		void insert(iterator where, iterator_type source_beg, iterator_type source_end) {
			const u64 size = difference(source_beg, source_end);

			if(size == 0) {
				return;
			}

			// first segment
			if(where == begin()) {
				segment* new_segment = allocate_segment(size);
				new_segment->size = size;

				new_segment->next = m_first_segment;
				m_first_segment = new_segment;

				construct_range(new_segment->data, source_beg, source_end);
				return;
			}

			// last segment
			if(where == end()) {
				segment* new_segment = allocate_segment(size);
				new_segment->size = size;

				new_segment->next = nullptr;
				where.m_segment->next = new_segment;
				m_current_segment = new_segment;

				construct_range(new_segment->data, source_beg, source_end);
				return;
			}

			//// segment edge
			//if(where.m_index == where.m_segment->size) {
			//	segment* new_segment = allocate_segment(size);
			//	new_segment->size = size;

		
			//	new_segment->next = where.m_segment->next;
			//	where.m_segment->next = new_segment;

			//	construct_range(new_segment->data, source_beg, source_end);
			//	return;
			//}

			segment* new_segment = allocate_segment(size);
			new_segment->size = size;
			new_segment->next = where.m_segment->next;

			if(where.m_segment->next == nullptr) {
				m_current_segment = new_segment;
			}

			where.m_segment->next = new_segment;
			
			u64 used_in_first = where.m_index + size;
			const bool can_fit_into_where = used_in_first <= where.m_segment->capacity;

			if(can_fit_into_where) {
				//std::cout << "one segment\n";
				u64 leftover_in_first = where.m_segment->capacity - used_in_first;

				for(u64 i = size; i-- > 0;) {
					std::construct_at(&new_segment->data[size - i - 1], where.m_segment->data[where.m_segment->capacity - i - 1]);
				}

				for(u64 i = leftover_in_first; i-- > 0;) {
					std::construct_at(&where.m_segment->data[i + where.m_index + size], where.m_segment->data[where.m_index + i]);
				}

				for(u64 i = 0; i < size; ++i) {
					std::construct_at(&where.m_segment->data[i + where.m_index], *source_beg);
					++source_beg;
				}
			}
			else {
				//std::cout << "two segments\n";

				const u64 first_segment_size = where.m_segment->capacity - where.m_index;
				const u64 second_segment_size = size - first_segment_size;

				for(u64 i = 0; i < first_segment_size; ++i) {
					std::construct_at(&new_segment->data[i + second_segment_size], where.m_segment->data[i + where.m_index]);
				}

				for(u64 i = 0; i < first_segment_size; ++i) {
					std::construct_at(&where.m_segment->data[i + where.m_index], *source_beg);
					++source_beg;
				}

				for(u64 i = 0; i < second_segment_size; ++i) {
					std::construct_at(&new_segment->data[i], *source_beg);
					++source_beg;
				}
			}
		}

		[[nodiscard]] auto size() const -> u64 {
			return m_size;
		}

		[[nodiscard]] auto begin() -> iterator {
			return { m_first_segment, 0 };
		}
		[[nodiscard]] auto end() -> iterator {
			return { m_current_segment, m_current_segment->size };
		}
	protected:
		void deallocate() {
			while(m_first_segment) {
				segment* temp = m_first_segment;
				m_first_segment = m_first_segment->next;

				temp->~segment();
				utility::free(temp);
			}
		}

		[[nodiscard]] static auto allocate_segment(u64 capacity) -> segment* {
			auto seg = static_cast<segment*>(utility::malloc(sizeof(segment)));
			seg->data = static_cast<type*>(utility::malloc(sizeof(type) * capacity));
			seg->size = 0;
			seg->next = nullptr;
			seg->capacity = capacity;
			return seg;
		}
	protected:
		segment* m_first_segment;
		segment* m_current_segment;

		u64 m_segment_capacity;
		u64 m_size; 
	};
} // namespace utility
