#include <iostream>

#include <chrono>
#include <set>
#include <map>

template<typename KEY, typename VALUE>
class LFUCache {
public:
    LFUCache(int cap) : _cap(cap), _time(0) {}

    void set(const KEY& key, const VALUE& value) {
        auto iter = _index.find(key);
        if (iter == _index.end()) {
            // 新增
            if (_data.size() >= _cap) {
                // 退场数据
                auto iter_del = _data.begin();
                _index.erase((*iter_del).key);
                _data.erase(iter_del);
            }
            Node node = {
                .key = key,
                .value = value,
                .timestamp = ++_time,
                .count = 0
            };
            auto iter_res = _data.emplace(node);
            _index.emplace(node.key, iter_res.first);
        } else {
            // 替换
            Node node = *(iter->second);
            _index.erase(node.key);
            _data.erase(node);
            node.value = value;
            auto iter_pos = _data.emplace(node).first;
            _index.emplace(node.key, iter_pos);
        }
    }

    bool get(const KEY& key, VALUE& value) {
        auto iter = _index.find(key);
        if (iter == _index.end()) {
            return false;
        }
        Node node = *(iter->second);
        _index.erase(node.key);
        _data.erase(node);
        node.count += 1;
        node.timestamp = ++_time;
        auto iter_pos = _data.emplace(node).first;
        _index.emplace(node.key, iter_pos);
        return true;
    }

private:
    struct Node {
        KEY key;
        VALUE value;
        uint64_t timestamp {0};
        int count {0};

        bool operator<(const LFUCache<KEY, VALUE>::Node& other) const {
            return count != other.count ?
                count < other.count : timestamp < other.timestamp;
        }
        // operator== is not used by std::set. Elements a and b are considered equal if !(a < b) && !(b < a)
        // equal的数据再set添加后会被认为是相同的数据而覆盖，因此timestamp使用递增计数避免相同
    };
private:
    std::set<Node> _data;
    std::map<KEY, typename std::set<Node>::iterator> _index;
    int _cap;
    uint64_t _time;
};
