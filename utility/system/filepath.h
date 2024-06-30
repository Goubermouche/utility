#pragma once
#include "../containers/dynamic_string.h"

namespace utility {
	class filepath {
	public:
		using size_type = u64;

		filepath() = default;
		filepath(const char* str) : m_data(str) {}
		filepath(const dynamic_string& str) : m_data(str) {}

		auto get_string() const -> const dynamic_string& {
			return m_data;
		}

		auto get_data() const -> const dynamic_string::element_type* {
			return m_data.get_data();
		}

		auto operator/=(const filepath& other) -> filepath& {
			if(m_data.get_last() != '/') {
				m_data += '/';
			}

			m_data.insert(m_data.end(), other.get_string().begin(), other.get_string().end());

			return *this;
		}

		auto get_filename() const -> filepath {
			const size_type pos = m_data.find_last_of('/');

			if(pos != dynamic_string::npos) {
				return { m_data.substring(pos + 1) };
			}

			return *this; // if no '/' found, return the entire path
		}

		auto get_extension() const -> filepath {
			const size_type pos = m_data.find_last_of('.');

			if(pos != dynamic_string::npos) {
				return { m_data.substring(pos) };
			}

			return {};
		}

		auto is_directory() const -> bool {
			return !m_data.is_empty() && m_data.get_last() == '/';
		}

		auto is_file() const -> bool {
			return !is_directory();
		}
	private:
		dynamic_string m_data;
	};

	template<typename stream_type>
	struct stream_writer<filepath, stream_type> {
		static void write(const filepath& value) {
			const auto& str = value.get_string();
			stream_type::write(str.get_data(), str.get_size());
		}
	};
} // namespace utility
