#pragma once
#include "../ranges.h"

namespace utility {
	template<typename iterator, typename compare>
	void sort(iterator begin, iterator end, const compare& comp) {
		if(distance(begin, end) <= 1) {
			return;
		}

		iterator pivot = begin + distance(begin, end) / 2;
		auto pivot_value = *pivot;

		iterator left = begin;
		iterator right = end - 1;

		while(left <= right) {
			while(comp(*left, pivot_value)) {
				++left;
			}

			while(comp(pivot_value, *right)) {
				--right;
			}

			if(left <= right) {
				swap(*left, *right);

				++left;
				--right;
			}
		}

		sort(begin, right + 1, comp);
		sort(left, end, comp);
	}
} // namespace utility
