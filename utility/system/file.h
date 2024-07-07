#pragma once
#include "../containers/dynamic_array.h"
#include "filepath.h"

namespace utility {
	struct directory {
		static auto collect_files(const filepath& root_path) -> dynamic_array<filepath> {
			if(!root_path.is_directory()) {
				return {}; // root_path is not a directory
			}

			dynamic_array<filepath> result;
			dynamic_array<dynamic_string_w> folders;
			const dynamic_string_w root_w = string_to_string_w(root_path.get_string());

			folders.push_back(string_to_string_w(root_path.get_string()));

			while(!folders.is_empty()) {
				const dynamic_string_w current = folders.pop_back();

				wchar_t temp_path[2048] = {0};
				WIN32_FIND_DATAW fdFile;
				HANDLE hFind;

				i32 written = swprintf(temp_path, 2048, L"%s/*.*", current.get_data());
				ASSERT(written > 0, "invalid path detected");

				if((hFind = FindFirstFileW(temp_path, &fdFile)) == INVALID_HANDLE_VALUE) {
					ASSERT(false, "Path '{}' not found", current);
					return {};
				}

				do {
					// find first file will always return "." and ".." as the first two directories.
					if(wcscmp(fdFile.cFileName, L".") != 0 && wcscmp(fdFile.cFileName, L"..") != 0) {
						written = swprintf(temp_path, 2048, L"%s/%s", current.get_data(), fdFile.cFileName);
						ASSERT(written > 0, "invalid path detected");

						if(fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
							// folder
							dynamic_string_w child_directory = temp_path;
							folders.push_back(child_directory);
						}
						else {
							// file
							dynamic_string_w file_path_w = temp_path;
							dynamic_string_w relative_path_w = file_path_w.substring(root_w.get_size() + 1); // Get path relative to root
							result.push_back(string_w_to_string(relative_path_w));
						}
					}
				} while(FindNextFileW(hFind, &fdFile)); // find the next file

				FindClose(hFind);
			}

			return result;
		}

		static auto exists(const filepath& path) -> bool {
			const DWORD attr = GetFileAttributesA(path.get_data());

			if(attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
				return true; // folder exists
			}

			return false; // folder does not exist or is not accessible
		}

		static void create_if_not_exists(const filepath& path) {
			if(exists(path)) {
				return;
			}

			if(!CreateDirectoryA(path.get_data(), nullptr)) {
				ASSERT(false, "failed to create directory");
			}
		}

		static void remove(const filepath& path) {
			if(!RemoveDirectory(string_to_string_w(path.get_string()).get_data())) {
				ASSERT(false, "failed to delete directory '{}'", path);
			}
		}
	};

	struct file {
		static auto exists(const filepath& path) -> bool {
			const DWORD attr = GetFileAttributesA(path.get_data());

			if(attr != INVALID_FILE_ATTRIBUTES) {
				return true;
			}

			return false;
		}

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
			// ensure parent directory exists
			directory::create_if_not_exists(path.get_parent_path());

			// open file for writing
			FILE* file;
			if(fopen_s(&file, path.get_data(), "w")) {
				ASSERT(false, "failed to open file '{}'", path.get_data());
			}

			if(file == nullptr) {
				ASSERT(false, "failed to write to file '{}'", path);
			}

			// write data to file
			if(fwrite(data.get_data(), sizeof(dynamic_string::element_type), data.get_size(), file) != data.get_size()) {
				ASSERT(false, "failed to write to file '{}'", path);

				if(fclose(file)) {
					ASSERT(false, "failed to close file '{}'", path);
				}

				return;
			}

			// close file
			if(fclose(file)) {
				ASSERT(false, "failed to close file '{}'", path);
			}
		}
	};
} // utility
