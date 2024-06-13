#pragma once
#include "dynamic_array.h"

namespace utility {
	class dynamic_string : public dynamic_array<char> {
	public:
		dynamic_string() = default;

		dynamic_string(const char* value) {
			reserve(std::strlen(value) + 1);
			utility::memset(m_data, 0, m_size * sizeof(char));
			m_size = m_capacity - 1;
			utility::memcpy(m_data, value, m_size);
		}

		[[nodiscard]] auto operator==(const char* other) const -> bool {
			const u64 len = std::strlen(other);

			if(len != m_size) {
				return false;
			}

			for(u64 i = 0; i < len; ++i) {
				if(other[i] != m_data[i]) {
					return false;
				}
			}

			return true;
		}
	};

	template<typename stream_type>
	struct stream_writer<dynamic_string, stream_type> {
		static void write(const dynamic_string& value) {
			stream_type::write(value.get_data(), value.get_size());
		}
	};
} // namespace utility
