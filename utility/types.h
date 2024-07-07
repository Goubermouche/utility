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
#include <string>

namespace utility {
	namespace types {
		struct integral {};
		struct floating_point {};

		template<typename type>
		concept is_integral = std::derived_from<type, integral>;

		template<typename type>
		concept is_floating_point = std::derived_from<type, floating_point>;

		// declare basic types
#define DECLARE_INTEGRAL_TYPE(name, underlying, min_v, max_v)                      \
    struct name : integral {                                                       \
      constexpr name() = default;                                                  \
      constexpr name(underlying v) : value(v) {}                                   \
      constexpr name(const name& b) = default;                                     \
      constexpr name(name&& b) noexcept : value(exchange(b.value, 0)) {}           \
                                                                                   \
      template<is_integral type>                                                   \
      explicit constexpr name(type v) : value(static_cast<underlying>(v.value)) {} \
                                                                                   \
      ~name() = default;                                                           \
                                                                                   \
      static constexpr auto max() -> name {                                        \
        return max_v;                                                              \
      }                                                                            \
                                                                                   \
      static constexpr auto min() -> name {                                        \
        return min_v;                                                              \
      }                                                                            \
                                                                                   \
      constexpr auto operator=(const name& b) -> name& {                           \
        if(this != &b) {                                                           \
          value = b.value;                                                         \
        }                                                                          \
        return *this;                                                              \
      }                                                                            \
      constexpr auto operator=(name&& b) noexcept -> name& {                       \
        if(this != &b) {                                                           \
          value = exchange(b.value, 0);                                            \
        }                                                                          \
        return *this;                                                              \
      }                                                                            \
                                                                                   \
      constexpr auto operator+=(underlying x) -> name& {                           \
        value += x;                                                                \
        return *this;                                                              \
      }                                                                            \
      constexpr auto operator-=(underlying x) -> name& {                           \
        value -= x;                                                                \
        return *this;                                                              \
      }                                                                            \
      constexpr auto operator*=(underlying x) -> name& {                           \
        value *= x;                                                                \
        return *this;                                                              \
      }                                                                            \
      constexpr auto operator%=(underlying x) -> name& {                           \
        value %= x;                                                                \
        return *this;                                                              \
      }                                                                            \
      constexpr auto operator&=(underlying x) -> name& {                           \
        value &= x;                                                                \
        return *this;                                                              \
      }                                                                            \
      constexpr auto operator|=(underlying x) -> name& {                           \
        value |= x;                                                                \
        return *this;                                                              \
      }                                                                            \
      constexpr auto operator^=(underlying x) -> name& {                           \
        value ^= x;                                                                \
        return *this;                                                              \
      }                                                                            \
      constexpr auto operator<<=(underlying x) -> name& {                          \
        value <<= x;                                                               \
        return *this;                                                              \
      }                                                                            \
      constexpr auto operator>>=(underlying x) -> name& {                          \
        value >>= x;                                                               \
        return *this;                                                              \
      }                                                                            \
      constexpr auto operator/=(underlying x) -> name& {                           \
        value /= x;                                                                \
        return *this;                                                              \
      }                                                                            \
                                                                                   \
      constexpr auto operator++() -> name& {                                       \
        ++value;                                                                   \
        return *this;                                                              \
      }                                                                            \
      constexpr auto operator++(int) -> name {                                     \
        name tmp(*this);                                                           \
        ++(*this);                                                                 \
        return tmp;                                                                \
      }                                                                            \
      constexpr auto operator--() -> name& {                                       \
        --value;                                                                   \
        return *this;                                                              \
      }                                                                            \
      constexpr auto operator--(int) -> name {                                     \
        name tmp(*this);                                                           \
        --(*this);                                                                 \
        return tmp;                                                                \
      }                                                                            \
      constexpr auto operator~() const -> name {                                   \
        return name(~value);                                                       \
      }                                                                            \
                                                                                   \
      constexpr operator underlying() const {                                      \
        return value;                                                              \
      }                                                                            \
                                                                                   \
      underlying value;                                                            \
    };

#define DECLARE_FLOATING_POINT_TYPE(name, underlying, min_v, max_v, eps_v)         \
    struct name {                                                                  \
      constexpr name() = default;                                                  \
      constexpr name(underlying v) : value(v) {}                                   \
      constexpr name(const name& b) = default;                                     \
      constexpr name(name&& b) noexcept : value(exchange(b.value, 0)) {}           \
                                                                                   \
      template<is_floating_point type>                                             \
      explicit constexpr name(type v) : value(static_cast<underlying>(v.value)) {} \
                                                                                   \
      ~name() = default;                                                           \
                                                                                   \
      static constexpr auto max() -> name {                                        \
        return max_v                  ;                                            \
      }                                                                            \
                                                                                   \
      static constexpr auto min() -> name {                                        \
        return min_v;                                                              \
      }                                                                            \
                                                                                   \
      static constexpr auto lowest() -> name {                                     \
        return -(max_v);                                                           \
      }                                                                            \
                                                                                   \
      static constexpr auto eps() -> name {                                        \
        return eps_v;                                                              \
      }                                                                            \
                                                                                   \
      constexpr auto operator=(const name& b) -> name& {                           \
        if(this != &b) {                                                           \
          value = b.value;                                                         \
        }                                                                          \
        return *this;                                                              \
      }                                                                            \
      constexpr auto operator=(name&& b) noexcept -> name& {                       \
        if(this != &b) {                                                           \
          value = exchange(b.value, 0);                                            \
        }                                                                          \
        return *this;                                                              \
      }                                                                            \
                                                                                   \
      constexpr auto operator+=(underlying x) -> name& {                           \
        value += x;                                                                \
        return *this;                                                              \
      }                                                                            \
      constexpr auto operator-=(underlying x) -> name& {                           \
        value -= x;                                                                \
        return *this;                                                              \
      }                                                                            \
      constexpr auto operator*=(underlying x) -> name& {                           \
        value *= x;                                                                \
        return *this;                                                              \
      }                                                                            \
      constexpr auto operator/=(underlying x) -> name& {                           \
        value /= x;                                                                \
        return *this;                                                              \
      }                                                                            \
                                                                                   \
      constexpr auto operator++() -> name& {                                       \
        ++value;                                                                   \
        return *this;                                                              \
      }                                                                            \
      constexpr auto operator++(int) -> name {                                     \
        name tmp(*this);                                                           \
        ++(*this);                                                                 \
        return tmp;                                                                \
      }                                                                            \
      constexpr auto operator--() -> name& {                                       \
        --value;                                                                   \
        return *this;                                                              \
      }                                                                            \
      constexpr auto operator--(int) -> name {                                     \
        name tmp(*this);                                                           \
        --(*this);                                                                 \
        return tmp;                                                                \
      }                                                                            \
                                                                                   \
      constexpr operator underlying() const {                                      \
        return value;                                                              \
      }                                                                            \
                                                                                   \
    private:                                                                       \
      underlying value;                                                            \
    };

		// unsigned integers
		DECLARE_INTEGRAL_TYPE(u8 , uint8_t , 0, 255                   )
		DECLARE_INTEGRAL_TYPE(u16, uint16_t, 0, 65535                 )
		DECLARE_INTEGRAL_TYPE(u32, uint32_t, 0, 4294967295            )
		DECLARE_INTEGRAL_TYPE(u64, uint64_t, 0, 0xffffffffffffffffui64)

		// signed integers
		DECLARE_INTEGRAL_TYPE(i8 , int8_t , -128                       , 127                   )
		DECLARE_INTEGRAL_TYPE(i16, int16_t, -32768                     , 32767                 )
		DECLARE_INTEGRAL_TYPE(i32, int32_t, -2147483648                , 2147483647            )
		DECLARE_INTEGRAL_TYPE(i64, int64_t, -9223372036854775807i64 - 1, 9223372036854775807i64)

		// floating point
		DECLARE_FLOATING_POINT_TYPE(f32, float , 1.175494351e-38F       , 3.402823466e+38F       , 1.192092896e-07F       )
		DECLARE_FLOATING_POINT_TYPE(f64, double, 2.2250738585072014e-308, 1.7976931348623158e+308, 2.2204460492503131e-016)

#undef DECLARE_INTEGRAL_TYPE
#undef DECLARE_FLOATING_POINT_TYPE
	} // namespace types

	using namespace types;

	template<typename type>
	using initializer_list = std::initializer_list<type>;

	// memory

	inline void free(void* data) {
		std::free(data);
	}
	inline auto malloc(u64 size) -> void* {
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

	inline auto string_len(const char* str) -> u64 {
		return std::strlen(str);
	}
	inline auto string_len(const wchar_t* str) -> u64 {
		return std::wcslen(str);
	}
	inline auto is_space(char c) noexcept -> bool {
		return (c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r' || c == ' ');
	}

	template<typename a, typename b = a>
	auto min(a left, b right) {
		return left > right ? right : left;
	}

	template<typename a, typename b = a>
	auto max(a left, b right) {
		return left > right ? left : right;
	}
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