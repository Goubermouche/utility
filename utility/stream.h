#pragma once
#include "types.h"

// TODO: f32's and f64's are currently broken

namespace utility {
	class stream {};

	// base stream writer

	template<typename type, typename stream_type>
	struct stream_writer {
		static void write(const type& value) {
			stream_type::write(value);
		}
	};

	// base stream writers for core types

	template<typename stream_type>
	struct stream_writer<char, stream_type> {
		static void write(char value) {
			static char buffer[2];
			buffer[0] = value;
			buffer[1] = '\0';

			stream_type::write(buffer);
		}
	};

	template<typename stream_type>
	struct stream_writer<bool, stream_type> {
		static void write(bool value) {
			stream_type::write(value ? "true" : "false");
		}
	};

	template<typename stream_type>
	struct stream_writer<i64, stream_type> {
		static void write(i64 value) {
			static char buffer[21];
			char* ptr = buffer + sizeof(buffer) - 1;
			i32 is_negative = 0;

			*ptr = '\0';

			if(value < 0) {
				is_negative = 1;
				value = -value;
			}

			do {
				*--ptr = static_cast<char>(48 + (value % 10));
				value /= 10;
			} while(value > 0);

			if(is_negative) {
				*--ptr = '-';
			}

			stream_type::write(ptr);
		}
	};

	template<typename stream_type>
	struct stream_writer<i32, stream_type> {
		static void write(i32 value) {
			static char buffer[12];
			char* ptr = buffer + sizeof(buffer) - 1;
			i32 is_negative = 0;

			*ptr = '\0';

			if(value < 0) {
				is_negative = 1;
				value = -value;
			}

			do {
				*--ptr = static_cast<char>(48 + (value % 10));
				value /= 10;
			} while(value > 0);

			if(is_negative) {
				*--ptr = '-';
			}

			stream_type::write(ptr);
		}
	};

	template<typename stream_type>
	struct stream_writer<i16, stream_type> {
		static void write(i16 value) {
			static char buffer[7];
			char* ptr = buffer + sizeof(buffer) - 1;
			i32 is_negative = 0;

			*ptr = '\0';

			if(value < 0) {
				is_negative = 1;
				value = -value;
			}

			do {
				*--ptr = static_cast<char>(48 + (value % 10));
				value /= 10;
			} while(value > 0);

			if(is_negative) {
				*--ptr = '-';
			}

			stream_type::write(ptr);
		}
	};

	template<typename stream_type>
	struct stream_writer<i8, stream_type> {
		static void write(i8 value) {
			static char buffer[5];
			char* ptr = buffer + sizeof(buffer) - 1;
			i32 is_negative = 0;

			*ptr = '\0';

			if(value < 0) {
				is_negative = 1;
				value = -value;
			}

			do {
				*--ptr = static_cast<char>(48 + (value % 10));
				value /= 10;
			} while(value > 0);

			if(is_negative) {
				*--ptr = '-';
			}

			stream_type::write(ptr);
		}
	};

	template<typename stream_type>
	struct stream_writer<u64, stream_type> {
		static void write(u64 value) {
			static char buffer[21];
			char* ptr = buffer + sizeof(buffer) - 1;

			*ptr = '\0';

			do {
				*--ptr = static_cast<char>(48 + (value % 10));
				value /= 10;
			} while(value > 0);

			stream_type::write(ptr);
		}
	};

	template<typename stream_type>
	struct stream_writer<u32, stream_type> {
		static void write(u32 value) {
			static char buffer[11];
			char* ptr = buffer + sizeof(buffer) - 1;

			*ptr = '\0';

			do {
				*--ptr = static_cast<char>(48 + (value % 10));
				value /= 10;
			} while(value > 0);

			stream_type::write(ptr);
		}
	};

	template<typename stream_type>
	struct stream_writer<u16, stream_type> {
		static void write(u16 value) {
			static char buffer[6];
			char* ptr = buffer + sizeof(buffer) - 1;

			*ptr = '\0';

			do {
				*--ptr = static_cast<char>(48 + (value % 10));
				value /= 10;
			} while(value > 0);

			stream_type::write(ptr);
		}
	};

	template<typename stream_type>
	struct stream_writer<u8, stream_type> {
		static void write(u8 value) {
			static char buffer[4];
			char* ptr = buffer + sizeof(buffer) - 1;

			*ptr = '\0';

			do {
				*--ptr = static_cast<char>(48 + (value % 10));
				value /= 10;
			} while(value > 0);

			stream_type::write(ptr);
		}
	};

	template<typename stream_type>
	struct stream_writer<f32, stream_type> {
		static void write(f32 value) {
			char buffer[20];
    	snprintf(buffer, sizeof(buffer), "%.2f", value);
			stream_type::write(buffer);
		}
	};

	template<typename stream_type>
	struct stream_writer<f64, stream_type> {
		static void write(f64 value) {
			char buffer[30];
    	snprintf(buffer, sizeof(buffer), "%.2f", value);
			stream_type::write(buffer);
		}
	};
} // namespace utility

