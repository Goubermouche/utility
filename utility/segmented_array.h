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
			iterator(segment* segment, u64 index) : segment(segment), index(index) {}

			auto operator++() -> iterator& {
				if(index + 1 >= segment->size && segment->next) {
					segment = segment->next;
					index = 0;
					return *this;
				}

				index++;
				return *this;
			}

			auto operator+(u64 offset) -> iterator& {
				for(u64 i = 0; i < offset; ++i) {
					this->operator++();
				}

				return *this;
			}

			auto operator*() -> type& {
				return segment->data[index];
			}

			auto operator==(const iterator& other) const -> bool {
				return segment == other.segment && index == other.index;
			}

			auto operator!=(const iterator& other) const -> bool {
				return !(*this == other);
			}

			segment* segment;
			u64 index;
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
			// allocate a new segment, if necessary 
			if(m_current_segment->size >= m_current_segment->capacity) {
				segment* next = allocate_segment(m_segment_capacity);
				m_current_segment->next = next;
				m_current_segment = next;
			}

			// append the new element
			if constexpr(std::is_trivial_v<type>) {
				m_current_segment->data[m_current_segment->size++] = value;
			}
			else {
				std::construct_at(&m_current_segment->data[m_current_segment->size++], value);
			}

			m_size++;
		}

		template<typename iterator_type>
		void insert(iterator where, iterator_type source_begin, iterator_type source_end) {
			const u64 size = utility::distance(source_begin, source_end);
			m_size += size;

			if(size == 0) {
				return; // no elements to insert, exit early
			}

			const u64 index = where.index;
			const u64 segment_capacity = where.segment->capacity;
			const u64 segment_size = where.segment->size;
			const u64 used_in_first = index + size;

			// check if we can fit our data into one segment without allocating a new one
			if(segment_size + size <= segment_capacity) {
				std::cout << "inplace\n";
				const u64 new_size = segment_size + size;
				const u64 size_of_moved = segment_size - index;
				
				for(u64 i = size_of_moved; i-- > 0;) {
					std::construct_at(&where.segment->data[index + i + size], where.segment->data[index + i]);
				}

				// insert the new elements
				for(u64 i = 0; i < size; ++i) {
					std::construct_at(&where.segment->data[index + i], *source_begin);
					++source_begin;
				}

				where.segment->size = new_size;
				return;
			}

			// allocate a new segment for the elements
			segment* new_segment = allocate_segment(size);
			new_segment->size = size;
			new_segment->next = where.segment->next;

			// update the current segment pointer if necessary
			if(where.segment->next == nullptr) {
				m_current_segment = new_segment;
			}

			where.segment->next = new_segment;
		
			// check if the new data fits within the current segment
			if(used_in_first <= segment_capacity) {
				std::cout << "single\n";
				const u64 leftover_in_first = segment_size - used_in_first;
				const u64 unpopulated_in_first = segment_capacity - segment_size;

				std::cout << unpopulated_in_first << '\n';

				// move existing elements to make space for the new elements
				for(u64 i = size; i > 0; --i) {
					std::construct_at(&new_segment->data[size - i], where.segment->data[segment_size - i]);
				}

				for(u64 i = leftover_in_first; i > 0; --i) {
					std::construct_at(&where.segment->data[index + size + i - 1], where.segment->data[index + i - 1]);
				}

				// insert the new elements
				for(u64 i = 0; i < size; ++i) {
					std::construct_at(&where.segment->data[index + i], *source_begin);
					++source_begin;
				}
			}
			else {
				std::cout << "double\n";
				// the new data spans across the current and the new segment
				const u64 first_segment_size = segment_size - index;
				const u64 second_segment_size = size - first_segment_size;

				// move elements to the new segment
				for(u64 i = 0; i < first_segment_size; ++i) {
					std::construct_at(&new_segment->data[second_segment_size + i], where.segment->data[index + i]);
				}

				// insert new elements in the first segment
				for(u64 i = 0; i < first_segment_size; ++i) {
					std::construct_at(&where.segment->data[index + i], *source_begin);
					++source_begin;
				}

				// insert remaining new elements in the new segment
				for(u64 i = 0; i < second_segment_size; ++i) {
					std::construct_at(&new_segment->data[i], *source_begin);
					++source_begin;
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
