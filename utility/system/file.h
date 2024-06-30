#pragma once
#include "../assert.h"
#include "filepath.h"

namespace utility {
	struct file {
		static auto read(const filepath& path) -> dynamic_string {
			FILE* file;
			dynamic_string result;

			if(fopen_s(&file, path.get_data(), "r")) {
				ASSERT(false, "failed to open file '{}'", path);
			}

			if(file == nullptr) {
				ASSERT(false, "file '{}' doesn't exist", path);
			}
			
			while(true) {
				constexpr u64 chunk_size = 1024;
				char buffer[chunk_size];
				const u64 bytes_read = fread(buffer, 1, chunk_size, file);

				if(bytes_read > 0) {
					result.insert(result.end(), buffer, buffer + bytes_read);
				}
				else {
					break;
				}
			}

			return result;
		}

		static void write(const filepath& path, const dynamic_string& data) {
			FILE* file;

			if(fopen_s(&file, path.get_data(), "w")) {
				ASSERT(false, "failed to open file '{}'", path);
			}

			if(file == nullptr) {
				ASSERT(false, "failed to write to file '{}'", path);
			}

			for(auto it = data.begin(); it != data.end(); ++it) {
				if(fwrite(it, sizeof(dynamic_string::element_type), 1, file) != 1) {
					ASSERT(false, "failed to write to file {}", path);

					if(fclose(file)) {
						ASSERT(false, "failed to close file {}", path);
					}

					return;
				}
			}

			if(fclose(file)) {
				ASSERT(false, "failed to close file {}", path);
			}
		}
	};
} // utility
