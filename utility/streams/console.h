#pragma once
#include "stream.h"

namespace utility {
	class console : public stream {
	private:
		friend class constructor;

		struct constructor {
			constructor() {
				stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
				stderr_handle = GetStdHandle(STD_ERROR_HANDLE);
			}

			HANDLE stdout_handle;
			HANDLE stderr_handle;
		};
	public:
		template<typename type>
		static void print(const type& value) {
			m_current_handle = m_value.stdout_handle;
			print_impl(value);
		}

		template<typename type>
		static void print_err(const type& value) {
			m_current_handle = m_value.stderr_handle;
			print_impl(value);
		}

		template<typename type, typename... types>
		static void print(const char* format, const type& first, const types&... rest) {
			m_current_handle = m_value.stdout_handle;
			print_impl(format, first, std::forward<const types>(rest)...);
		}

		template<typename type, typename... types>
		static void print_err(const char* format, const type& first, const types&... rest) {
			m_current_handle = m_value.stderr_handle;
			print_impl(format, first, std::forward<const types>(rest)...);
		}

		static void write(const char* data) {
			write(data, std::strlen(data));
		}

		static void write(const char* data, u64 size) {
			DWORD bytes_written;
			WriteConsoleA(m_current_handle, data, static_cast<DWORD>(size), &bytes_written, nullptr);
		}

		static void flush() {
			FlushFileBuffers(m_current_handle);
		}
	protected:
		template<typename type, typename... types>
		static void print_impl(const char* format, const type& first, const types&... rest) {
			if(const char* open_brace = std::strstr(format, "{}")) {
				write(format, open_brace - format);
				stream_writer<type, console>::write(first);
				print_impl(open_brace + 2, rest...);
			}
			else {
				write(format);
			}
		}

		template<typename type>
		static void print_impl(const type& value) {
			stream_writer<type, console>::write(value);
		}
	protected:
		static HANDLE m_current_handle;
		static constructor m_value; // static constructor
	};

	inline HANDLE console::m_current_handle;
	inline console::constructor console::m_value;
} // namespace utility
