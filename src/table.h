#include <unordered_map>
#include <mutex>

// 简单的有锁哈希存储

template <typename KEY, typename VALUE>
class Table {
public:
    // 添加kv
    // return: true - 成功; false - 失败，key重复
    bool put(const KEY& key, const VALUE& value);

    // 通过key获得value
    // return: true - 成功，value填入对应值; false - 失败，value保留原值
    bool get(const KEY& key, VALUE& value);

    // 删除kv
    void erase(const KEY& key);

    // 批量退场, 参数为key*避免拷贝
    void batch_erase(const std::vector<const KEY*>& pkeys);

    uint64_t size();

private:
    std::mutex _mutex;
    std::unordered_map<KEY, VALUE> _table;
};

template <typename KEY, typename VALUE>
bool Table<KEY, VALUE>::put(const KEY& key, const VALUE& value) {
    std::lock_guard<std::mutex> guard(_mutex);
    return _table.emplace(key, value).second;	
}

template <typename KEY, typename VALUE>
bool Table<KEY, VALUE>::get(const KEY& key, VALUE& value) {
    std::lock_guard<std::mutex> guard(_mutex);

    auto it = _table.find(key);
    if (it == _table.end()) {
        return false;
    }
	
    value = it->second;
    return true;
}

template <typename KEY, typename VALUE>
void Table<KEY, VALUE>::erase(const KEY& key) {
    std::lock_guard<std::mutex> guard(_mutex);
    _table.erase(key);
}

template <typename KEY, typename VALUE>
void Table<KEY, VALUE>::batch_erase(const std::vector<const KEY*>& pkeys) {
    std::lock_guard<std::mutex> guard(_mutex);
    for (const auto& pkey : pkeys) {
        _table.erase(*pkey);
    }
}

template <typename KEY, typename VALUE>
uint64_t Table<KEY, VALUE>::size() {
    std::lock_guard<std::mutex> guard(_mutex);
    return _table.size();
}
