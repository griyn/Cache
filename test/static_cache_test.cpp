#include <iostream>
#include <string>
#include <memory>
#include "test_tool.h"
#include "static_cache.h"

int main() {
	griyn::StaticCache<int, std::string> qu(3);

	// 初始化容量
	EXPECT_EQ(qu.capacity(), 3);
	
	// 重复添加同一个key和value
	EXPECT_EQ(qu.put(1, "Hello1"), 0);
	EXPECT_EQ(qu.put(1, "Hello1"), 1); // 发现添加了重复的key
	EXPECT_EQ(qu.size(), 1);
	
	// 添加相同key，不同value
	EXPECT_EQ(qu.put(1, "World1"), 1);
	EXPECT_EQ(qu.size(), 1);
	std::string output;
	EXPECT_EQ(qu.get(1, output), 0);
	EXPECT_EQ(output, "World1");
	EXPECT_EQ(qu.size(), 1); // 依然只有一个元素

	// 添加新key
	EXPECT_EQ(qu.put(2, "Hello2"), 0);
	EXPECT_EQ(qu.size(), 2);
	EXPECT_EQ(qu.get(2, output), 0);
	EXPECT_EQ(output, "Hello2");

	// 获得不存在的key
	EXPECT_EQ(qu.get(3, output), 1);

	// 超过容量
	EXPECT_EQ(qu.put(3, "Hello3"), 0); // 1 2 3
	EXPECT_EQ(qu.put(4, "Hello4"), 0); // 1 已被删除
	EXPECT_EQ(qu.size(), 3);	
	EXPECT_EQ(qu.get(1, output), 1); // 1 已经查不到了
	EXPECT_EQ(qu.get(2, output), 0); // 2 3 4 在队列中
	EXPECT_EQ(output, "Hello2");
	EXPECT_EQ(qu.get(3, output), 0);
	EXPECT_EQ(output, "Hello3");
	EXPECT_EQ(qu.get(4, output), 0);
	EXPECT_EQ(output, "Hello4");

	return 0;
}
