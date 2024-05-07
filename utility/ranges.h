#pragma once
#include "types.h"

namespace utility {
	template <typename destination_it_type, typename source_it_type>
	void construct_range(destination_it_type destination, source_it_type begin, source_it_type end) {
		while(begin != end) {
			std::construct_at(destination, *begin);
			++destination;
			++begin;
		}
	}

	template <typename iterator_type>
	void destruct_range(iterator_type begin, iterator_type end) {
		while(begin != end) {
			std::destroy_at(begin);
			++begin;
		}
	}

	template<typename iterator_type>
	auto difference(iterator_type begin, iterator_type end) -> u64 {
		u64 diff = 0;

		while(begin != end) {
			++begin;
			diff++;
		}

		return diff;
	}

	template<typename iterator_type>
	void iterator_print(iterator_type begin, iterator_type end) {
		std::cout << '{';

		while(begin != end) {
			std::cout << *begin;
			++begin;

			if(begin != end) {
				std::cout << ", ";
			}
			else {
				std::cout << "}\n";
				return;
			}
		}
	}
} // namespace utility
