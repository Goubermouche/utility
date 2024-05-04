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
} // namespace utility
