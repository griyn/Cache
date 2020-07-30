#include <string>
#include <memory>
#include "expire_cache.h"
#include "test_tool.h"

int main() {
	griyn::ExpireCache<uint32_t, std::string> cache(2);
	
	EXPECT_EQ(cache.put(1, "Hello"), true);
	EXPECT_EQ(cache.put(1, "Hello"), false); // 添加重复key
	EXPECT_EQ(cache.put(2, "World"), true);
	EXPECT_EQ(cache.size(), 2);
	EXPECT_EQ(cache.timeq_size(), 2);

	std::string output;
	EXPECT_EQ(cache.get(1, output), true);
	EXPECT_EQ(output, "Hello");
	EXPECT_EQ(cache.get(2, output), true);
	EXPECT_EQ(output, "World");
	EXPECT_EQ(cache.get(3, output), false); // 获取不存在的key
	EXPECT_EQ(output, "World"); // output没有被修改
	EXPECT_EQ(cache.size(), 2);
	EXPECT_EQ(cache.timeq_size(), 2);

	std::this_thread::sleep_for(std::chrono::seconds(3)); // 过期，拿不到数据
	EXPECT_EQ(cache.get(1, output), false);
	EXPECT_EQ(output, "World");
	EXPECT_EQ(cache.get(2, output), false);
	EXPECT_EQ(output, "World");
	EXPECT_EQ(cache.size(), 0);
	EXPECT_EQ(cache.timeq_size(), 0);

	return 0;
}
