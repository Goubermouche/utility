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
			iterator() = default;
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
		segmented_array()
		: m_first_segment(nullptr), m_current_segment(nullptr), m_segment_capacity(0), m_size(0) {}

		segmented_array(u64 segment_size)
		: m_first_segment(nullptr), m_current_segment(nullptr), m_segment_capacity(segment_size), m_size(0) {
			m_first_segment = allocate_segment(m_segment_capacity);
			m_current_segment = m_first_segment;
		}

		segmented_array(u64 segment_size, u64 size)
		: m_first_segment(nullptr), m_current_segment(nullptr), m_segment_capacity(segment_size), m_size(size) {
			m_first_segment = allocate_segment(size);

			m_current_segment = m_first_segment;
			m_first_segment->size = size;
		}

		segmented_array(u64 segment_size, u64 size, const type& value)
			: m_first_segment(nullptr), m_current_segment(nullptr), m_segment_capacity(segment_size), m_size(size) {
			m_first_segment = allocate_segment(size);

			m_current_segment = m_first_segment;
			m_first_segment->size = size;

			for(u64 i = 0; i < size; ++i) {
				std::construct_at(&m_current_segment->data[i], value);
			}
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
			u64 data_size = utility::distance(source_begin, source_end);
			m_size += data_size;

			if(data_size == 0) {
				return; // no elements to insert, exit early
			}

			const u64 start = where.index;
			const u64 segment_capacity = where.segment->capacity;
			const u64 segment_size = where.segment->size;

			// check if we can fit our data into one segment without allocating a new one
			if(segment_size + data_size <= segment_capacity) {
				const u64 new_size = segment_size + data_size;
				const u64 size_of_moved = segment_size - start;
				
				for(u64 i = size_of_moved; i-- > 0;) {
					std::construct_at(&where.segment->data[start + i + data_size], where.segment->data[start + i]);
				}

				// insert the new elements
				for(u64 i = 0; i < data_size; ++i) {
					std::construct_at(&where.segment->data[start + i], *source_begin);
					++source_begin;
				}

				where.segment->size = new_size;
				return;
			}

			const u64 unused_size = segment_capacity - segment_size;
			const u64 additional_size = data_size - unused_size;

			// allocate a new segment for the elements
			segment* new_segment = allocate_segment(additional_size);
			new_segment->size = additional_size;

			// update the first segment if necessary 
			if(where == begin()) {
				m_first_segment = new_segment;
				new_segment->next = where.segment;

				for(u64 i = 0; i < data_size; ++i) {
					std::construct_at(&new_segment->data[i], *source_begin);
					++source_begin;
				}

				return;
			}

			// update the current segment pointer if necessary
			if(where.segment->next == nullptr) {
				m_current_segment = new_segment;
			}

			// update the linked list
			new_segment->next = where.segment->next;
			where.segment->next = new_segment;
			where.segment->size = std::min(segment_capacity, segment_size + data_size);

			// move old data
			segment* target = new_segment;
			u64 target_index = target->size - 1;

			for(u64 index = segment_size; index-- > start; ) {
				target->data[target_index] = where.segment->data[index];

				if(target_index == 0) {
					target = where.segment;
					target_index = where.segment->size - 1;
				}
				else {
					--target_index;
				}
			}

			// insert new elements
			if(start + data_size <= segment_capacity) {
				// we can fit everything into one segment
				for(u64 i = 0; i < data_size; ++i) {
					std::construct_at(&where.segment->data[start + i], *source_begin);
					++source_begin;
				}
			}
			else {
				// we have to split everything into two segments 
				const u64 first_segment_size = segment_capacity - start;
				const u64 second_segment_size = data_size - first_segment_size;

				for(u64 i = 0; i < first_segment_size; ++i) {
					std::construct_at(&where.segment->data[start + i], *source_begin);
					++source_begin;
				}

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
