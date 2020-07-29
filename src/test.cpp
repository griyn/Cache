#include <map>
#include <string>
#include <stdio.h>

int main() {
	std::map<int, std::string> m;
	std::string str = "Hello";
	m.insert({1, str});

	printf("%X\n", str);
	printf("%X\n", m[1]);
	return 0;
}
