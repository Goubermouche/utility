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
			// handle negative values
			if(value < 0) {
				value = -value;
			}

			// extract the exponent
			i32 exponent = 0;

			if(value != 0.0f) {
				exponent = static_cast<i32>(std::floor(std::log10(value)));

				if(exponent < -3 || exponent > 3) {
					value /= std::pow(10.0f, exponent);
				}
				else {
					exponent = 0;
				}
			}

			// convert the mantissa to a string
			i32 int_part = static_cast<i32>(value);
			f32 fraction = value - static_cast<f32>(int_part);
			static char buffer[100];
			u8 index = 0;

			stream_writer<i32, stream_type>::write(int_part);

			// fractional part
			if(fraction > 0.0f) {
				buffer[index++] = '.';

				for(u8 i = 0; i < 4; i++) {
					fraction *= 10;
					const u8 digit = static_cast<u8>(fraction);
					buffer[index++] = static_cast<char>(digit + 48);
					fraction -= static_cast<f32>(digit);
				}

				// remove trailing zeros
				while(index > 0 && buffer[index - 1] == '0') {
					index--;
				}

				// ensure there's at least one digit after the decimal point
				if(buffer[index - 1] == '.') {
					index--;
				}
			}

			// append the exponent, if necessary
			if(exponent != 0) {
				buffer[index++] = 'e';

				if(exponent < 0) {
					buffer[index++] = '-';
					exponent = -exponent;
				}
				else {
					buffer[index++] = '+';
				}

				if(exponent < 10) {
					buffer[index++] = '0';
				}

				buffer[index] = '\0';
				stream_type::write(buffer);
				stream_writer<i32, stream_type>::write(exponent);
			}
			else {
				buffer[index] = '\0';
			}
		}
	};

	template<typename stream_type>
	struct stream_writer<f64, stream_type> {
		static void write(f64 value) {
			// handle negative values
			if(value < 0) {
				value = -value;
			}

			// extract the exponent
			i32 exponent = 0;

			if(value != 0.0) {
				exponent = static_cast<i32>(std::floor(std::log10(value)));

				if(exponent < -3 || exponent > 3) {
					value /= std::pow(10.0, exponent);
				}
				else {
					exponent = 0;
				}
			}

			// convert the mantissa to a string
			i32 int_part = static_cast<i32>(value);
			f64 fraction = value - int_part;
			static char buffer[100];
			u8 index = 0;

			stream_writer<i32, stream_type>::write(int_part);

			// fractional part
			if(fraction > 0.0) {
				buffer[index++] = '.';

				for(u8 i = 0; i < 4; i++) {
					fraction *= 10;
					const u8 digit = static_cast<u8>(fraction);
					buffer[index++] = static_cast<char>(digit + 48);
					fraction -= digit;
				}

				// remove trailing zeros
				while(index > 0 && buffer[index - 1] == '0') {
					index--;
				}

				// ensure there's at least one digit after the decimal point
				if(buffer[index - 1] == '.') {
					index--;
				}
			}

			// append the exponent, if necessary
			if(exponent != 0) {
				buffer[index++] = 'e';

				if(exponent < 0) {
					buffer[index++] = '-';
					exponent = -exponent;
				}
				else {
					buffer[index++] = '+';
				}

				if(exponent < 10) {
					buffer[index++] = '0';
				}

				buffer[index] = '\0';
				stream_type::write(buffer);
				stream_writer<i32, stream_type>::write(exponent);
			}
			else {
				buffer[index] = '\0';
			}
		}
	};

	template<typename stream_type>
	struct stream_writer<int8_t, stream_type> {
		static void write(int8_t value) {
			stream_writer<i8, stream_type>::write(i8(value));
		}
	};

	template<typename stream_type>
	struct stream_writer<int16_t, stream_type> {
		static void write(int16_t value) {
			stream_writer<i16, stream_type>::write(i16(value));
		}
	};

	template<typename stream_type>
	struct stream_writer<int32_t, stream_type> {
		static void write(int32_t value) {
			stream_writer<i32, stream_type>::write(i32(value));
		}
	};

	template<typename stream_type>
	struct stream_writer<int64_t, stream_type> {
		static void write(int64_t value) {
			stream_writer<i64, stream_type>::write(i64(value));
		}
	};

	template<typename stream_type>
	struct stream_writer<uint8_t, stream_type> {
		static void write(uint8_t value) {
			stream_writer<u8, stream_type>::write(u8(value));
		}
	};

	template<typename stream_type>
	struct stream_writer<uint16_t, stream_type> {
		static void write(uint16_t value) {
			stream_writer<u16, stream_type>::write(u16(value));
		}
	};

	template<typename stream_type>
	struct stream_writer<uint32_t, stream_type> {
		static void write(uint32_t value) {
			stream_writer<u32, stream_type>::write(u32(value));
		}
	};

	template<typename stream_type>
	struct stream_writer<uint64_t, stream_type> {
		static void write(uint64_t value) {
			stream_writer<u64, stream_type>::write(u64(value));
		}
	};
} // namespace utility
