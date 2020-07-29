// 顺序cache
// 	cache大小固定, 移除规则fifo，避免内存膨胀。
//	适用于用户使用value做决策的场景，cache通常需要设到足够大。

#include <map>
#include <list>
#include <tuple>
#include <mutex>

namespace griyn { // griyn

template <typename KEY, typename VALUE>
class StaticCache {
public:
	StaticCache(uint32_t capacity) : _cap(capacity) {};

	// 添加
	// return: 0-添加的是新数据
	// 		   1-更新了旧数据
	int put(const KEY& key, const VALUE& value);

	// 获取
	// 不更新热数据
	// return: 0-队列中存在, output中的是取出的数据；
	// 		   1-队列中不存在, output没有被赋值
	int get(const KEY& key, VALUE& output);

	uint32_t capacity() { return _cap; }
	uint32_t size() { return _store.size(); };

private:
	uint32_t _cap;
	std::map<KEY, typename std::list<std::pair<KEY, VALUE>>::iterator> _index;
	std::list<std::pair<KEY, VALUE>> _store;
	std::mutex _mutex;
};

////// implememt //////
template <typename KEY, typename VALUE>
int StaticCache<KEY, VALUE>::put(const KEY& key, const VALUE& value) {
	int ret = 0;

	std::lock_guard<std::mutex> op_guard(_mutex);
	auto index_find = _index.find(key);
	if (index_find != _index.end()) {
		// key 已存在，删除原数据，重新添加到队首
		_store.erase(index_find->second);
		ret = 1;
	}

	_store.push_front({key, value});  // kv 发生拷贝
	_index.insert({key, _store.begin()}); // k 拷贝

	// 删除超过容量的队尾数据
	if (size() > capacity()) {
		_index.erase((--_store.end())->first);
		_store.pop_back();
	}

	return ret;
}

template <typename KEY, typename VALUE>
int StaticCache<KEY, VALUE>::get(const KEY& key, VALUE& output) {
	std::lock_guard<std::mutex> op_guard(_mutex);
	auto index_find = _index.find(key);
	if (index_find == _index.end()) {
		return 1;
	}

	output = index_find->second->second; // v 拷贝
	return 0;
}

} // namespace griyn
