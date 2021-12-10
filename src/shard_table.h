#include <functional> // std::hash
#include <vector>
#include "table.h"

// 分片化的哈希存储结构
// 通过分片减少读写竞争

template <typename KEY, typename VALUE>
class ShardTable {
public:
    ShardTable(int32_t shard_num);

    // 添加kv
    // return: true - 成功; false - 失败，key重复
    bool put(const KEY& key, const VALUE& value);

    // 通过key获得value
    // return: true - 成功，value填入对应值; false - 失败，value保留原值
    bool get(const KEY& key, VALUE& value);

    // 删除kv
    void erase(const KEY& key);

    // 批量退场 for ExpireCache
    void batch_erase(const std::vector<KEY>& keys);

    uint64_t size();

private:
    // 生成分片id的方法
    uint32_t get_shard_id(const KEY& key);

private:
    std::vector<Table<KEY, VALUE>> _shards;
};

template <typename KEY, typename VALUE>
ShardTable<KEY, VALUE>::ShardTable(int32_t shard_num) :
        _shards(shard_num) {
}

template <typename KEY, typename VALUE>
bool ShardTable<KEY, VALUE>::put(const KEY& key, const VALUE& value) {
    return _shards[get_shard_id(key)].put(key, value);	
}

template <typename KEY, typename VALUE>
bool ShardTable<KEY, VALUE>::get(const KEY& key, VALUE& value) {
    return _shards[get_shard_id(key)].get(key, value);
}

template <typename KEY, typename VALUE>
void ShardTable<KEY, VALUE>::erase(const KEY& key) {
    return _shards[get_shard_id(key)].erase(key);
}

template <typename KEY, typename VALUE>
void ShardTable<KEY, VALUE>::batch_erase(const std::vector<KEY>& keys) {
    std::vector<std::vector<const KEY*>> erase_pkeys(_shards.size());
    for (size_t i = 0; i < keys.size(); ++i) {
        const KEY* pkey = &keys[i];
        erase_pkeys[get_shard_id(keys[i])].push_back(pkey);
    }

    for (size_t i = 0; i < erase_pkeys.size(); ++i) {
        const auto& pkeys = erase_pkeys[i];
        if (pkeys.size() != 0) {
            _shards[i].batch_erase(pkeys);
        }
    }
}

template <typename KEY, typename VALUE>
uint64_t ShardTable<KEY, VALUE>::size() {
    uint64_t size = 0;
    for (auto& shard : _shards) {
        size += shard.size();
    }

    return size;
}

template <typename KEY, typename VALUE>
uint32_t ShardTable<KEY, VALUE>::get_shard_id(const KEY& key) {
    return std::hash<KEY>()(key) % _shards.size();	
}
