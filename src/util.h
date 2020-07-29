#include <chrono>


uint32_t now_s() {
	return std::chrono::duration_cast<std::chrono::seconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count();	
}
