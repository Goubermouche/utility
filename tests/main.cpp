#include "utility/map.h"

int main() {
	utility::map<int, int> map;

	map[10] = 20;

	for(const auto& [key, value] : map) {
		std::cout << "[" << key << ", " << value << "]\n";
	}

	return 0;
}