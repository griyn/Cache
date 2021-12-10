#pragma once

#include <list>
#include <map>
#include <mutex>

template <typename KEY, typename VALUE>
class LRUCache {
public:
    typedef std::pair<KEY, VALUE> Data;
    typedef std::list<Data> List;
    typedef std::map<KEY, typename List::const_iterator> Index;

public:
    LRUCache(int cap) : _cap(cap) {}

    bool get(const KEY& key, VALUE& value) {
        std::lock_guard<std::mutex> guard(_mutex);
        auto it = _index.find(key);
        // key不存在
        if (it == _index.end()) {
            return false;
        }
        // key存在，调整时间序列，返回获得的值
        move_front(it->second);
        value = it->second->second;
        return true;
    }

    void put(const KEY& key, const VALUE& value) {
        std::lock_guard<std::mutex> guard(_mutex);
        auto it = _index.find(key);
        // 元素已存在，调整时间序列，设定新value
        if (it != _index.end()) {
            move_front(it->second);
            _time_queue.begin()->second = value;
            return;
        }
        // 添加了不存在的元素
        if (_index.size() >= _cap) {
            pop_back();
        }
        put_front(key, value);

        return;
    }
    
    void last(KEY& key) {
        std::lock_guard<std::mutex> guard(_mutex);
        if (_index.size() > 0) {
            key = _time_queue.begin()->first;
        }
    }

private:
    // 把节点移动到队首
    void move_front(typename List::const_iterator& it) {
        // 把it移动到list的头部，无拷贝
        // (移动给本list的位置，from list，from list的节点)
        _time_queue.splice(_time_queue.begin(), _time_queue, it);
    }

    // 在队首添加新节点
    void put_front(const KEY& key, const VALUE& value) {
        _time_queue.push_front({key, value});
        _index.emplace(key, _time_queue.begin());
    }

    // 删除队尾数据
    void pop_back() {
        const KEY& key = _time_queue.back().first;
        _index.erase(key);
        _time_queue.pop_back();
    }

private:
    int _cap;
    std::mutex _mutex;
    List _time_queue; // 数据存储结构，按时间顺序保存，方面淘汰数据
    Index _index; // 索引
};
