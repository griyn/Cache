#include <unordered_map>
#include <functional> // std::hash
#include <vector>
#include <memory>
#include <mutex>

template <typename KEY, typename VALUE>
class ShardTable {
public:
    typedef std::unordered_map<KEY, VALUE> ShardType;

    ShardTable(uint32_t shard_num);

    // return
    //  true - 成功；false - 失败，key重复
    bool put(const KEY& key, const VALUE& value);

    // return
    //  true - 成功，value填入对应值；false - 失败，value保留原值
    bool get(const KEY& key, VALUE& value);

    void erase(const KEY& key);
    void batch_erase(const std::vector<KEY>& keys);

    uint64_t size();

private:
    uint32_t get_shard_id(const KEY& key);

private:
    uint32_t _shard_num;
    std::mutex _mutex;
    std::vector<std::unique_ptr<ShardType>>	_shards;
};

template <typename KEY, typename VALUE>
ShardTable<KEY, VALUE>::ShardTable(uint32_t shard_num) :
        _shard_num(shard_num) {
    for (int i = 0; i < _shard_num; ++i) {
        _shards.emplace_back(new ShardType);
    }
}

template <typename KEY, typename VALUE>
bool ShardTable<KEY, VALUE>::put(const KEY& key, const VALUE& value) {
    std::lock_guard<std::mutex> op_guard(_mutex);
    return _shards[get_shard_id(key)]->insert({key, value}).second;	
}

template <typename KEY, typename VALUE>
bool ShardTable<KEY, VALUE>::get(const KEY& key, VALUE& value) {
    std::lock_guard<std::mutex> op_guard(_mutex);
    uint32_t sid = get_shard_id(key);

    auto it = _shards[sid]->find(key);
    if (it == _shards[sid]->end()) {
        return false;
    }
	
    value = it->second;
    return true;
}

template <typename KEY, typename VALUE>
void ShardTable<KEY, VALUE>::erase(const KEY& key) {
    std::lock_guard<std::mutex> op_guard(_mutex);
    _shards[get_shard_id(key)]->erase(key);
}

template <typename KEY, typename VALUE>
void ShardTable<KEY, VALUE>::batch_erase(const std::vector<KEY>& keys) {
    std::lock_guard<std::mutex> op_guard(_mutex);
    for (const auto& key : keys) {
        _shards[get_shard_id(key)]->erase(key);
    }
}

template <typename KEY, typename VALUE>
uint64_t ShardTable<KEY, VALUE>::size() {
    std::lock_guard<std::mutex> op_guard(_mutex);
	
    uint64_t size = 0;
    for (const auto& shard : _shards) {
        size += shard->size();
    }

    return size;
}

template <typename KEY, typename VALUE>
uint32_t ShardTable<KEY, VALUE>::get_shard_id(const KEY& key) {
    return std::hash<KEY>()(key) % _shard_num;	
}
