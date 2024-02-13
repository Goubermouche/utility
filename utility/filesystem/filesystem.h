#pragma once
#include "../memory/contiguous_memory.h"
#include "filepath.h"

namespace utility {
	namespace fs {
		template<typename type, typename size_type>
		auto write(const filepath& path, const contiguous_memory<type, size_type>& value) -> result<void> {
			std::ofstream file(path.get_path(), std::ios::binary);

			if(!file) {
				return error(std::format("cannot write file {}\n", path));
			}

			for (const type* ptr = value.begin(); ptr != value.end(); ++ptr) {
				file.write(reinterpret_cast<const char*>(ptr), sizeof(type));
			}

			file.close();

			return SUCCESS;
		}

		inline auto load(const filepath& path) -> result<std::string> {
			std::ifstream file(path.get_path());

			if(!file) {
				return error(std::format("cannot open file '{}'", path));
			}

			return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		}

		class directory {
		public:
			static void for_all_directory_items(const filepath& path, const std::function<void(const filepath&)>& function) {
				ASSERT(path.exists(), "directory '{}' doesn't exist", path);
				ASSERT(path.is_directory(), "filepath '{}' doesn't point to a directory", path);

				for (const auto& entry : std::filesystem::directory_iterator(path.get_path())) {
					function(entry.path());
				}
			}
		};

		inline auto remove(const filepath& path) -> bool {
			return std::filesystem::remove(path.get_path());
		}

		inline void create(const filepath& path) {
			// TODO: add error checking
			std::ofstream file(path.get_path());
			file.close();
		}
	}
} // namespace utility::fs
