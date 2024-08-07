#pragma once
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#elif __linux__
#include <unistd.h>
#endif

#include "./type_traits.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <cmath>

#include <initializer_list>

namespace utility {
	namespace types {
		// unsigned integers
		using u8  = uint8_t;
		using u16 = uint16_t;
		using u32 = uint32_t;
		using u64 = uint64_t;

		// signed integers
		using i8 = int8_t;
		using i16 = int16_t;
		using i32 = int32_t;
		using i64 = int64_t;

		// floating point
		using f32 = float;
		using f64 = double;
	} // namespace types

	using namespace types;

	template<typename type>
	using initializer_list = std::initializer_list<type>;

	// memory

	inline void free(void* data) {
		std::free(data);
	}
	[[nodiscard]] inline auto malloc(u64 size) -> void* {
		return std::malloc(size);
	}
	inline void memcpy(void* destination, const void* source, u64 size) {
		std::memcpy(destination, source, size);
	}
	inline void memset(void* destination, i32 value, u64 size) {
		std::memset(destination, value, size);
	}
	inline void memmove(void* destination, const void* source, u64 size) {
		std::memmove(destination, source, size);
	}

	// strings

	[[nodiscard]] inline auto string_len(const char* str) -> u64 {
		return std::strlen(str);
	}
	[[nodiscard]] inline auto string_len(const wchar_t* str) -> u64 {
		return std::wcslen(str);
	}
	[[nodiscard]] inline auto is_space(char c) noexcept -> bool {
		return (c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r' || c == ' ');
	}

	template<typename a, typename b = a>
	[[nodiscard]] auto min(a left, b right) {
		return left > right ? right : left;
	}

	template<typename a, typename b = a>
	[[nodiscard]] auto max(a left, b right) {
		return left > right ? left : right;
	}

	template<typename type>
	struct limits {};

	template<>
	struct limits<u8> {
		[[nodiscard]] static constexpr auto max() noexcept -> u8 {
			return 255;
		}

		[[nodiscard]] static constexpr auto min() noexcept -> u8 {
			return 0;
		}
	};

	template<>
	struct limits<u16> {
		[[nodiscard]] static constexpr auto max() noexcept -> u16 {
			return 65535;
		}

		[[nodiscard]] static constexpr auto min() noexcept -> u16 {
			return 0;
		}
	};

	template<>
	struct limits<u32> {
		[[nodiscard]] static constexpr auto max() noexcept -> u32 {
			return 4294967295;
		}

		[[nodiscard]] static constexpr auto min() noexcept -> u32 {
			return 0;
		}
	};

	template<>
	struct limits<u64> {
		[[nodiscard]] static constexpr auto max() noexcept -> u64 {
			return 18446744073709551615ull;
		}

		[[nodiscard]] static constexpr auto min() noexcept -> u64 {
			return 0;
		}
	};

	template<>
	struct limits<i8> {
		[[nodiscard]] static constexpr auto max() noexcept -> i8 {
			return 127;
		}

		[[nodiscard]] static constexpr auto min() noexcept -> i8 {
			return -128;
		}
	};

	template<>
	struct limits<i16> {
		[[nodiscard]] static constexpr auto max() noexcept -> i16 {
			return 32767;
		}

		[[nodiscard]] static constexpr auto min() noexcept -> i16 {
			return -32768;
		}
	};

	template<>
	struct limits<i32> {
		[[nodiscard]] static constexpr auto max() noexcept -> i32 {
			return 2147483647;
		}

		[[nodiscard]] static constexpr auto min() noexcept -> i32 {
			return -2147483648;
		}
	};

	template<>
	struct limits<i64> {
		[[nodiscard]] static constexpr auto max() noexcept -> i64 {
			return 9223372036854775807;
		}

		[[nodiscard]] static constexpr auto min() noexcept -> i64 {
			return -9223372036854775808i64;
		}
	};
} // namespace utility

#define EXPAND(__x) __x

#ifdef _MSC_VER
#define GET_ARG_COUNT(...)  INTERNAL_EXPAND_ARGS_PRIVATE(INTERNAL_ARGS_AUGMENTER(__VA_ARGS__))
#define INTERNAL_ARGS_AUGMENTER(...) unused, __VA_ARGS__
#define INTERNAL_EXPAND_ARGS_PRIVATE(...) EXPAND(INTERNAL_GET_ARG_COUNT_PRIVATE(__VA_ARGS__, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define INTERNAL_GET_ARG_COUNT_PRIVATE(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count
#else
#define GET_ARG_COUNT(...) INTERNAL_GET_ARG_COUNT_PRIVATE(0, ## __VA_ARGS__, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define INTERNAL_GET_ARG_COUNT_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count
#endif

#define CONCATENATE_INDIRECT(__x, __y) __x ## __y
#define CONCATENATE(__x, __y) CONCATENATE_INDIRECT(__x, __y)

#define SUPPRESS_C4100(__value) (void)__value