#pragma once
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#elif __linux__
#include <unistd.h>
#endif

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <tchar.h>
#include <cmath>

#include <initializer_list>
#include <chrono>
#include <utility>

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

		using word  = u16;
		using qword = u64;
	} // namespace types

	using namespace types;

	template<typename type>
	using initializer_list = std::initializer_list<type>;

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
	inline auto string_len(const char* str) -> u64 {
		return std::strlen(str);
	}
	inline auto string_len(const wchar_t* str) -> u64 {
		return std::wcslen(str);
	}

	template<typename type>
	void swap(type& left, type& right) noexcept {
		type temp = std::move(left);
		left = std::move(right);
		right = std::move(temp);
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

	class timer {
	public:
		timer() {
			reset();
		}

		void reset() {
			m_start = std::chrono::high_resolution_clock::now();
		}

		void resume() {
			m_start = std::chrono::high_resolution_clock::now();
		}

		template<typename time_unit>
		auto get_elapsed() -> f64 {
			m_elapsed += std::chrono::high_resolution_clock::now() - m_start;
			return static_cast<f64>(std::chrono::duration_cast<time_unit>(m_elapsed).count());
		}
	private:
		std::chrono::high_resolution_clock::time_point m_start;
		std::chrono::duration<f64, std::micro> m_elapsed = std::chrono::high_resolution_clock::duration::zero();
	};

	class byte {
	public:
		constexpr byte() : m_value(0) {}
		constexpr byte(u8 value) : m_value(value) {}

		[[nodiscard]] constexpr operator u8() const { return m_value; }
		[[nodiscard]] constexpr auto operator<=>(const byte& other) const->std::strong_ordering = default;
		[[nodiscard]] constexpr auto operator~() const -> byte { return ~m_value; }
		[[nodiscard]] constexpr auto operator&(const byte& other) const -> byte { return m_value & other.m_value; }
		[[nodiscard]] constexpr auto operator|(const byte& other) const -> byte { return m_value | other.m_value; }
		[[nodiscard]] constexpr auto operator^(const byte& other) const -> byte { return m_value ^ other.m_value; }
		[[nodiscard]] constexpr auto operator<<(i32 shift) const -> byte { return static_cast<u8>(m_value << shift); }
		[[nodiscard]] constexpr auto operator>>(i32 shift) const -> byte { return m_value >> shift; }
		[[nodiscard]] friend auto operator+(const byte& a, const byte& b) -> byte { return a.m_value + b.m_value; }
	private:
		u8 m_value;
	};

	class dword {
	public:
		constexpr dword() : m_value(0) {}
		constexpr dword(u32 value) : m_value(value) {}

		[[nodiscard]] constexpr operator u32() const { return m_value; }
		[[nodiscard]] constexpr auto operator<=>(const dword& other) const->std::strong_ordering = default;
		[[nodiscard]] constexpr auto operator~() const -> dword { return ~m_value; }
		[[nodiscard]] constexpr auto operator&(const dword& other) const -> dword { return m_value & other.m_value; }
		[[nodiscard]] constexpr auto operator|(const dword& other) const -> dword { return m_value | other.m_value; }
		[[nodiscard]] constexpr auto operator^(const dword& other) const -> dword { return m_value ^ other.m_value; }
		[[nodiscard]] constexpr auto operator<<(i32 shift) const -> dword { return static_cast<u8>(m_value << shift); }
		[[nodiscard]] constexpr auto operator>>(i32 shift) const -> dword { return m_value >> shift; }
	private:
		u32 m_value;
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