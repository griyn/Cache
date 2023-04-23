# Cache

## ExpireCache
* 定时批量清理过期数据
* 数据结构：顺序时间队列 + 分片存储
  * 时间队列做索引
  * 哈希分片存储，减少锁冲突
  * 仅支持全局 ttl；无法更新已存储数据的过期时间
  
## StaticCache
* 静态Cache，用户自己选择添加、删除数据
* 大于max_size添加数据时，移除最早添加的数据

## TODO
ExpiredCache中的时间队列有点意义不明，无法作为一种通用组件，只能支持当前轮子。数据索引和时间队列分别维护，导致退场时效率低。

因此设想了更合理的实现方式：
```
template <typename KEY, typename VALUE>
class ExpiredCache {
public:
    void expire_work() { // per sec
        guard(_shard_mtx);
        _cur_node 转换到 _timed_queue
        while (!_timed_queue.empty()) {
            退场所有过期node
            退场对应索引
        }
    }
private:
    TimedNode<KEY> _cur_node;
    std::list<TimedNode<KEY>> _timed_queue; // 时间队列，每秒一个node，退场粒度
    std::map<KEY, VALUE> _table; // 时间队列和索引放在一个shard，索引也可以再分片减少锁粒度
    std::thread _expire_worker;
    bthread::Mutex _shard_mtx;
    int64_t _shared_id;
}

class ExpiredCacheShared; // 管理ExpiredCache分片，减少锁粒度
```
