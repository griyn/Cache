#include <unordered_map>
#include <thread>
#include <memory>
#include "timed_queue.h"

namespace griyn {

template <typename KEY, typename VALUE>
class ExpireCache {
public:
	ExpireCache(
			uint32_t ttl_s, uint64_t capacity = -1, // TODO:size 
			uint32_t timer_interval_s = 1) :
		_ttl_s(ttl_s), _cap(capacity), 
		_timer_interval_s(timer_interval_s), _running(true),
		_expire_timer(&ExpireCache<KEY, VALUE>::timer_work, this) {}

	~ExpireCache();

	// return:
	// 		0 - 添加成功; 1 - 添加失败，key 重复
	int put(const KEY& key, const VALUE& value);

	// return:
	// 		0 - 查找成功，value 有值；1 - 查找失败，value 未被赋值
	int get(const KEY& key, VALUE& value);

	uint64_t size();

	// 需要遍历竞争较大，debug用
	uint64_t timeq_size();	

private:
	void timer_work();
	void pop_front();

private:
	uint32_t _ttl_s;
	uint64_t _cap;
	uint32_t _timer_interval_s;

	std::thread _expire_timer;
	bool _running;

	std::unordered_map<KEY, VALUE> _table; // TODO: shard
	
	// 将 1s 内的 key 保存在一个 node 里，定期清理(timer_interval)
	TimedQueue<KEY> _timed_queue;
};

////// IMPLEMENT //////
template <typename KEY, typename VALUE>
int ExpireCache<KEY, VALUE>::put(const KEY& key, const VALUE& value) {
	// 不能允许添加重复 key
	// 原因是 time_queue 不太好实现唯一 key
	//  其实可以在 table 里加个引用计数解决，先作为todo吧
	auto it = _table.find(key);
	if (it != _table.end()) {	
		return 1;
	}

	_timed_queue.put(key); // 保证时间队列 key 不重复
	_table.insert({key, value});

	return 0;
}

template <typename KEY, typename VALUE>
int ExpireCache<KEY, VALUE>::get(const KEY& key, VALUE& value) {
	auto it = _table.find(key);
	if (it == _table.end()) {
		return 1;
	}

	value = it->second;
	return 0;
}

template <typename KEY, typename VALUE>
void ExpireCache<KEY, VALUE>::timer_work() {
	std::this_thread::sleep_for(std::chrono::seconds(_timer_interval_s));

	while (_running) {
		uint32_t start = now_s();

		std::vector<KEY> expired_keys = _timed_queue.pop(_ttl_s);
		// TODO:batch erase
		for (auto& key : expired_keys) {
			_table.erase(key);
		}

		uint32_t end = now_s();

		std::this_thread::sleep_for(
				std::chrono::seconds(_timer_interval_s - (end - start)));
	}
}

template <typename KEY, typename VALUE>
ExpireCache<KEY, VALUE>::~ExpireCache() {
	_running = false;
	_expire_timer.join();
}

template <typename KEY, typename VALUE>
uint64_t ExpireCache<KEY, VALUE>::size() {
	return _table.size();
}

template <typename KEY, typename VALUE>
uint64_t ExpireCache<KEY, VALUE>::timeq_size() {
	return _timed_queue.size();
}

} // griyn
