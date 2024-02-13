// string_table utility header

#pragma once
#include "../macros.h"

namespace utility {
	struct string_table_key {
		string_table_key() : m_value(0) {}
		string_table_key(const std::string& string) : m_value(std::hash<std::string>{}(string)) {}

		auto operator==(string_table_key other) const -> bool {
			return m_value == other.m_value;
		}

		auto operator<(string_table_key other) const -> bool {
			return m_value < other.m_value;
		}

		/**
		 * \brief Returns the hashed string value.
		 * \return Hashed string value.
		 */
		auto get_value() const -> u64 {
			return m_value;
		}

		/**
		 * \brief Checks whether the key has been initialized to a valid value.
		 * \return True if the key is valid, false otherwise.
		 */
		auto is_valid() const -> bool {
			return m_value != 0;
		}
	private:
		u64 m_value; // already hashed
	};
} // namespace utility

template <>
struct std::hash<utility::string_table_key> {
	auto operator()(const utility::string_table_key& key) const noexcept -> utility::u64 {
		// since we've already hashed the string in the constructor we can use the value
		return key.get_value();
	}
};

namespace utility {
	class string_table {
	public:
		string_table() = default;

		/**
		 * \brief Checks if the table contains a given \b key.
		 * \param key Key to check
		 * \return True if \b key is contained within the table, false otherwise.
		 */
		auto contains(string_table_key key) const -> bool {
			return m_key_to_string.contains(key);
		}

		/**
		 * \brief Inserts \b string into the table and returns a key to it, if the string is already
		 * contained the key of that string is returned.
		 * \param string String to insert
		 * \return Key to the string.
		 */
		auto insert(const std::string& string) -> string_table_key {
			const string_table_key new_key(string);

			const auto it = m_key_to_string.find(new_key);
			if (it != m_key_to_string.end()) {
				// the key is already contained in the table
				return new_key;
			}

			m_key_to_string[new_key] = string;
			return new_key;
		}

		/**
		 * \brief Retrieves the specified \b key as a string value.
		 * \param key Key to retrieve
		 * \return String value stored under the specified \b key. 
		 */
		auto get(string_table_key key) const -> const std::string& {
			ASSERT(key.get_value() != 0, "invalid symbol key");
			return m_key_to_string.at(key);
		}
	private:
		std::unordered_map<string_table_key, std::string> m_key_to_string;
	};
} // namespace utility
