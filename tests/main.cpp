#include "utility/map.h"

using namespace utility::types;

int main() {
	utility::map<int, int> map;


	for (u64 i = 0; i < 100; ++i) {
		map[i] = i;
	}

	for(const auto& [key, value] : map) {
		std::cout << "[" << key << ", " << value << "]\n";
	}

	return 0;
}
