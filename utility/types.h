#pragma once
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace utility {
	namespace types {
		using u8  = uint8_t;
		using u16 = uint16_t;
		using u32 = uint32_t;
		using u64 = uint64_t;

		using i8  = int8_t;
		using i16 = int16_t;
		using i32 = int32_t;
		using i64 = int64_t;

		using f32 = float;
		using f64 = double;
	} // namespace types

	using namespace types;

	inline void free(void* data) {
		std::free(data);
	}
	inline auto malloc(u64 size) -> void* {
		return std::malloc(size);
	}
	inline void memcpy(void* destination, const void* source, u64 size) {
		std::memcpy(destination, source, size);
	}

	template<std::three_way_comparable type>
	struct range {
		constexpr range() : start(type()), end(type()) {}
		constexpr range(const type& start, const type& end) : start(start), end(end) {}

		[[nodiscard]] constexpr auto contains(const range& other) const -> bool {
			return start <= other.start && end >= other.end;
		}

		[[nodiscard]] constexpr auto overlaps(const range& other) const -> bool {
			return start < other.end && end > other.start;
		}

		[[nodiscard]] constexpr auto operator<=>(const range& other) const -> std::strong_ordering = default;

		friend auto operator<<(std::ostream& stream, const range& range) -> std::ostream& {
			stream << range.start << ':' << range.end;
			return stream;
		}

		type start;
		type end;
	};
} // namespace utility
