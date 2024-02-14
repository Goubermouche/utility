// string_view utility header

#pragma once
#include "../memory/contiguous_memory.h"
#include "../allocators/allocator_base.h"

namespace utility {
	namespace detail {
		template<typename char_type>
		auto string_length(const char_type* ptr) -> u64 {
			return static_cast<u64>(std::strlen(ptr));
		}
	} // namespace utility

	/**
	 * \brief A non-owning view of a sequence of characters. 
	 * \tparam char_type Type of the character element
	 * \tparam size_type Size type
	 */
	template<typename char_type = char, typename size_type = u64>
	class string_view : public contiguous_memory<char_type, size_type> {
	public:
		using base_type = contiguous_memory<char_type, size_type>;

		string_view() = delete;
		string_view(char_type* str, size_type size) : base_type(str, size) {}

		/**
		 * \brief Constructs a string using the provided allocator
		 * \tparam allocator Allocator type to use. Must derive from allocator_base.
		 * \param alloc Allocator to use
		 * \param str String to initialize with
		 */
		template<typename allocator>
		requires is_allocator<allocator>
		string_view(allocator& alloc, const char_type* str)
			: base_type(nullptr, detail::string_length(str)) {
			this->m_data = static_cast<char_type*>(alloc.allocate(this->m_size * sizeof(char_type)));
			std::memcpy(this->m_data, str, this->m_size * sizeof(char_type));
		}

		/**
		 * \brief Constructs a string using the provided std::string.
		 * \tparam allocator Allocator type to use. Must derive from allocator_base.
		 * \param alloc Allocator to use
		 * \param str String to initialize with
		 */
		template<typename allocator>
		requires is_allocator<allocator>
		string_view(allocator& alloc, const std::basic_string<char_type>& str)
			: base_type(nullptr, str.size()) {
			this->m_data = static_cast<char_type*>(alloc.allocate(this->m_size * sizeof(char_type)));
			std::memcpy(this->m_data, str.data(), this->m_size * sizeof(char_type));
		}

		auto operator==(const string_view& other) const -> bool {
			if(this->m_size == other.m_size) {
				if (this->m_data == other.m_data) {
					return true;
				}

				// fall back to comparing the individual elements
				for(size_type i = 0; i < this->m_size; ++i) {
					if(this->m_data[i] != other.m_data[i]) {
						return false;
					}
				}

				return true;
			}
		
			return false;
		}

		friend auto operator<<(std::ostream& os, const string_view& str) -> std::ostream& {
			for(size_type i = 0; i < str.m_size; ++i) { os << str.m_data[i]; }
			return os;
		}

		auto get_view(size_type start, size_type size) const -> string_view {
			return { this->m_data + start, size };
		}
	};
} // namespace utility

template<typename char_type, typename size_type>
struct std::formatter<utility::string_view<char_type, size_type>> : std::formatter<std::string> {
	auto format(const utility::string_view<char_type, size_type>& str, format_context& ctx) const {
		return std::format_to(ctx.out(), "{:.{}}", str.get_data(), str.get_size());
	}
};

namespace utility {
	/**
	 * \brief Creates a backslash-escaped version of the \a input string.
	 * \param input String to escape.
	 * \return Escaped version of the given string.
	 */
	inline auto escape_string(const std::string& input) -> std::string {
		// TODO: update to support various hexadecimal and binary strings
		std::string output;

		for (const char ch : input) {
			if (ch == '\\' || ch == '\'' || ch == '\"' || ch == '\a' || ch == '\b' || ch == '\f' || ch == '\n' || ch == '\r' || ch == '\t' || ch == '\v' || ch == '\x1b') {
				output.push_back('\\');
				switch (ch) {
					case '\\': output.push_back('\\'); break;
					case '\'': output.push_back('\''); break;
					case '\"': output.push_back('\"'); break;
					case '\a': output.push_back('a'); break;
					case '\b': output.push_back('b'); break;
					case '\f': output.push_back('f'); break;
					case '\n': output.push_back('n'); break;
					case '\r': output.push_back('r'); break;
					case '\t': output.push_back('t'); break;
					case '\v': output.push_back('v'); break;
					case '\x1b':
						output.append("x1b");
						break;
					}
			}
			else {
				output.push_back(ch);
			}
		}
		return output;
	}

	inline auto is_only_char(const std::string& s, char c) -> bool {
		for (const char ch : s) {
			if (ch != c) {
				return false;
			}
		}

		return true;
	}

	template<typename type>
	auto unsigned_from_string(const std::string& string, bool& overflow) -> type {
		overflow = false;
		bool is_negative = false;
		type result = 0;

		u64 start_index = 0;
		if (!string.empty() && string[0] == '-') {
			is_negative = true;
			overflow = true;
			start_index = 1;
		}

		for (u64 i = start_index; i < string.length(); ++i) {
			const char ch = string[i];

			if (!std::isdigit(ch)) {
				overflow = true;
				return result;
			}

			i32 digit = ch - '0';

			// check for overflow
			if (result > (std::numeric_limits<type>::max() - digit) / 10) {
				overflow = true;
				result = (result * 10 + digit) & std::numeric_limits<type>::max();
			}
			else {
				result = result * 10 + static_cast<type>(digit);
			}
		}

		// handle underflow by converting to max value for negative inputs
		if (is_negative) {
			return std::numeric_limits<type>::max() - result + 1;
		}

		return result;
	}

	/**
	 * \brief Converts \b str to type, allows overflow behavior, when overflow occurs the \b overflow
	 * flag is set. It's expected that \b str contains a valid value for \b type.
	 * \tparam type Type to convert string to
	 * \param string Str to parse
	 * \param overflowed Overflow flag
	 * \return \b str parsed as \b type.
	 */
	template<typename type>
	type from_string(const std::string& string, bool& overflowed) {
		static_assert(
			std::is_integral_v<type> || std::is_floating_point_v<type>,
			"'type' must be integral or floating point"
			);

		overflowed = false;

		if constexpr (std::is_integral_v<type>) {
			if constexpr (std::is_same_v<type, i8> || std::is_same_v<type, u8>) {
				i32 value;
				std::istringstream stream(string);

				stream >> value;
				overflowed = stream.fail() || value > std::numeric_limits<type>::max() || value < std::numeric_limits<type>::min();
				return static_cast<type>(value);
			}
			else if constexpr (std::is_unsigned_v<type>) {
				return unsigned_from_string<type>(string, overflowed);
			}
			else if constexpr (std::is_signed_v<type>) {
				type value;
				std::istringstream stream(string);

				stream >> value;
				overflowed = stream.fail() || value > std::numeric_limits<type>::max() || value < std::numeric_limits<type>::min();
				return value;
			}
		}
	}
} // namespace utility
