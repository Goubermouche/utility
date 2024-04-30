#include "utility/map.h"

using namespace utility::types;

int main() {
	utility::map<int, int> map;

	for(i32 i = 0; i < 100; ++i) {
		map[i + 100] = i;
	}

	auto other = map;

	for(const auto& [key, value] : other) {
		std::cout << "[" << key << ", " << value << "]\n";
	}

	return 0;
}
