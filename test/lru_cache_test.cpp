#include "test_tool.h"
#include "lru_cache.h"
#include <string>

int main() {
    LRUCache<std::string, std::string> lru(3);
    lru.put("Jessica", "17");
    lru.put("George", "25");
    lru.put("Battler", "17");
    std::string output;
    EXPECT_EQ(lru.get("Jessica", output), true);
    EXPECT_EQ(output, "17");

    lru.last(output);
    EXPECT_EQ(output, "Jessica");

    lru.put("Maria", "9");
    EXPECT_EQ(lru.get("George", output), false);

    return 0;
}
