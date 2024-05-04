#include "utility/map.h"
#include "utility/assert.h"
#include "utility/string.h"
#include "utility/allocators/block_allocator.h"

using namespace utility::types;

int main() {
	utility::map<int, int> map;

	for(i32 i = 0; i < 100; ++i) {
		map[i + 100] = i;
	}

	auto other = std::move(map);

	for(const auto& [key, value] : other) {
		std::cout << "[" << key << ", " << value << "]\n";
	}

	utility::block_allocator allocator(100);



	for(int i = 0; i < 100; ++i) {
		auto x = allocator.emplace<int>();
	}

	std::cout << "creating safepoint\n";
	const auto safepoint = allocator.create_safepoint();

	for(int i = 0; i < 100; ++i) {
		auto x = allocator.emplace<int>();
	}

	std::cout << "restoring safepoint\n";
	allocator.restore_safepoint(safepoint);

	for(int i = 0; i < 300; ++i) {
		auto x = allocator.emplace<int>();
	}

	return 0;
}
