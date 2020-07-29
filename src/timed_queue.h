#pragma once

#include <memory>
#include <vector>
#include <list>
#include <mutex>
#include "util.h"

namespace griyn {

template<typename T>
struct TimeNode {
	uint32_t timestamp_s;
	std::vector<T> keys;
	TimeNode(uint32_t time_s) : timestamp_s(time_s) {}	
};

template<typename T>
class TimedQueue {
public:
	TimedQueue() : _cur_node(new TimeNode<T>(now_s())) {};

	void put(const T& t);
	
	// 刷入节点并且 pop 所有过期数据
	std::vector<T> pop(uint32_t ttl);

	// 查看状态，debug用，需要遍历很耗时
	uint64_t size();

private:
	void flush_cur_node();

private:
	std::mutex _lock;

	// 将 1s 内的 key 保存在一个 node 里，定期清理(timer_interval)
	std::unique_ptr<TimeNode<T>> _cur_node; // 避免在node入队发生拷贝，使用了指针
	std::list<std::unique_ptr<TimeNode<T>>> _queue; // back添加，front删除
};

template<typename T>
void TimedQueue<T>::put(const T& t) {
	std::lock_guard<std::mutex> op_guard(_lock);

	// 由pop操作保证_cur_node可用
	// 这里不会关注 T 在整个队列中是否重复
	_cur_node->keys.push_back(t);
}

template<typename T>
std::vector<T> TimedQueue<T>::pop(uint32_t ttl) {
	std::lock_guard<std::mutex> op_guard(_lock);

	uint32_t now = now_s();
	// 刷入新节点
	flush_cur_node();

	// 返回所有超时节点 pop
	std::vector<T> expired_keys;
	while (!_queue.empty()) {
		auto it = _queue.cbegin(); // the expression c.front() is equivalent to *c.begin().
		if ((*it)->timestamp_s + ttl > now) {
			break; // 到达未超时节点
		} 
		expired_keys.insert(expired_keys.end(), (*it)->keys.begin(), (*it)->keys.end());
		_queue.pop_front();
	}

	return expired_keys;
}

template<typename T>
void TimedQueue<T>::flush_cur_node() {
	// 如果没有添加新key，则不刷入节点，重置cur时间
	if (_cur_node->keys.empty()) {
		_cur_node->timestamp_s = now_s();
		return;
	}

	_queue.push_back(std::move(_cur_node));
	_cur_node.reset(new TimeNode<T>(now_s()));
}

template<typename T>
uint64_t TimedQueue<T>::size() {
	std::lock_guard<std::mutex> op_guard(_lock);
	uint64_t size = _cur_node->keys.size();

	for (const auto& node : _queue) {
		size += node->keys.size();
	}

	return size;
}

} // griyn
