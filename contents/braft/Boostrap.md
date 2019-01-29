启动Options

struct BootstrapOptions {
    // 当前raft group的初始成员，默认是空的
    Configuration group_conf
    
    // 当前正在dump的快照里面最后的index
    int64_t last_log_index;

    // 用来dump第一个快照的状态机，如果last_log_index > 0 这个必须设置
    StateMachine* fsm;

    // 当前节点是不是要独占一个状态机
    bool node_owns_fsm;

    // 是不是在线程中调用用户自己的回调，默认是在bthread中调用
    bool usercode_in_pthread;

    // LogStorage
    std::string log_uri;

    // RaftMetaStorage
    std::string raft_meta_uri;

    // SnapshotStorage
    std::string snapshot_uri;
}

从一个例子入手braft
================


一个Server，
一个ServiceImpl，是某个服务接口的实现
一个状态机的子类，Counter， // **Q0：状态机如何运作的？？**
Counter是ServiceImpl的实现，ServiceImpl是通过AddService加入到Server中的。

大体流程
=======
* server.start()提供服务。
* counter.start()启动状态机
  * 状态机里面包含一个Node，启动的时候Node被构造，初始化。
  * **Q1：Node的启动都需要什么？？**
* counter.shutdown()关闭状态机
* server.shutdown()关闭服务
* counter.join() 等着
* server.join() 等着



Q0：状态机都干啥了？？
==================

A0-1：状态机暴露给外部的接口
------------------------

braft中的状态机on_*接口都是非线程安全的，而且是顺序的被调用的，也就是某个操作存在hang住后面的情况

``` c++
class StateMachine {
    // 通过迭代器拿到那些被commited的tasks，然后更新状态机。
    // 当加入到Node::apply的一个或者多个tasks，被raft group 提交之后
    //（group中的大多数接到这些tasks并且存储他们到持久存储中）该接口会被调用
    on_apply(iter)

    // node shutdown的时候被调用
    on_shutdown()

    // 用户自定义快照生成函数，这个方法会阻塞on_apply，
    // 当fsm支持cow时，用户可以异步生成快照
    // 快照生成完成以后需要调用done->Run()
    on_snapshot_save(writer, done);
    
    // 提供给用户自定义读取加载快照函数
    on_snapshot_load(SnapshotReader reader);
    
    // 当当前node节点成为leader的时候会被调用
    on_leader_start(term);

    // 当当前node节点从leader位置退下来的时候会调用
    on_leader_stop(status);

    // 当遇到一个关键性的错误的时候，这个接口被调用，这个点结束以后，任何修改都不被允许应用到这个节点，直到这个错误恢复并且这个节点重启。
    on_error(&e);

    // 当一个配置信息被提交到当前group的时候会被调用
    on_configuration_committed(conf);

    // 当一个follower停止追随一个leader并且follower的leader_id = NULL的时候该方法被调用。
    // 包括以下情形：
    // 1. 选举超时并且启动一个pre_vote
    // 2. 从一个candidate接到一个vote_request请求，这个请求带的term号更高
    // 3. 从当前leader接到一个timeout_now_request，并开始一个voit_request
    on_stop_following(leader_change_ctx);

    // 当一个follower或者candidate开始追随一个leader的时候这个方法被调用
    // 这个方法被调用前需要保证leader_id = NULL
    // 包括下面情形：
    // 1. 一个candidate 从leader处接到一个append_entries请求
    // 2. 一个follower从leader处接到append_entries请求
    on_start_following(loader_change_ctx);
}
```

A0-2：状态机内部的实现
-------------------
状态机由FSMCaller驱动
FSMCaller在 Node初始化的时候 NodeImpl::init(const NodeOptions& options)会
1. new FSMCaller()
2. 调用int FSMCaller::init(const FSMCallerOptions &options)初始化
   1. 通过options吧_fsm传入到FSMCaller实例，对应本例子中，用户定义的状态机子类Counter
   2. 将FSMCaller::run方法加入到执行队列中，并启动，bthread::execution_queue_start  **Q3: 执行队列是什么鬼??**
   3. 在FSMCaller::run方法中
      1. 如果队列停了do_shutdown
      2. 枚举所有任务
         1. 如果任务是COMMITTED，更新max_commited_index，设置当前任务状态，调用 do_committed
         2. 


Q1: Node的启动都需要什么？？
========================

Node 是一个raft节点，在raft内部节点被彼此叫成Peer，每个Peer被分到一个id

GroupId 是一个string，唯一标识一个raft group

NodeId，全局上唯一定位到一个raft 节点
NodeId {
    GroupId
    PeerId
}

PeerId，在一个raft group内唯一定位一个节点
PeerId {
    addr; // ip端口地址
    idx;  // 同地址下支持多个peer
}


Node 初始化快照定时器SnapshotTimer，
1. 然后在定时器超时的时候调用对应的SnapshotTimer::run()
2. 调用NodeImpl::handle_snapshot_timeout()
3. 调用对应的NodeImpl::do_snapshot()
4. 调用_snapshot_executor的do_snapshot方法
   1. 判断是否已经stop了
   2. 判断是否正在加载另一个快照
   3. 判断是否正在保存另一个快照
   4. 

Node 维护着什么
-------------
1. State _state; 当前节点状态
2. int64_t _current_term; 当前term号
3. int64_t _last_leader_timestamp;
4. PeerId _leader_id;  
5. PeerId _voted_id; 投出去的票的id，就是Peer的Id
6. Ballot _vote_ctx;
7. Ballot _pre_vote_ctx;
8. ConfigurationEntry _conf;
9. GroupId _group_id; 当前raft group标识
10. PeerId _server_id; 当前peer的id
11. NodeOptions _options; //一些选项
12. raft_mutex_t _mutex;
13. ConfigurationCtx _conf_ctx;
14. LogStorage* _log_storage; // 日志引擎
15. RaftMetaStorage* _meta_storage; // 元信息引擎
16. ClosureQueue* _closure_queue; 
17. ConfigurationManager* _config_manager;
18. LogManager* _log_manager; // 日志管理器
19. FSMCaller* _fsm_caller;  
20. BallotBox* _ballot_box;  // 投票箱
21. SnapshotExecutor* _snapshot_executor;
22. ReplicatorGroup _replicator_group;
23. std::vector<Closure*> _shutdown_continuations;

24. 各种计时器：选举，投票，快照，leader转换
    1.  ElectionTimer _election_timer;  
    2.  VoteTimer _vote_timer;          
    3.  StepdownTimer _stepdown_timer;  
    4.  SnapshotTimer _snapshot_timer;  
    5.  bthread_timer_t _transfer_timer;  

25. StopTransferArg* _stop_transfer_arg;
26. ReplicatorId _waking_candidate;
27. bthread::ExecutionQueueId<LogEntryAndClosure> _apply_queue_id;
28. bthread::ExecutionQueue<LogEntryAndClosure>::scoped_ptr_t _apply_queue;
29. AppendEntriesCache* _append_entries_cache;
30. int64_t _append_entries_cache_version;4


Q3: 执行队列是什么鬼??
===================
类似于kylin的ExecMan, [ExecutionQueue](https://github.com/brpc/brpc/blob/master/src/bthread/execution_queue.h)提供了异步串行执行的功能。

- 异步有序执行: 任务在另外一个单独的线程中执行, 并且执行顺序严格和提交顺序一致.
- Multi Producer: 多个线程可以同时向一个ExecutionQueue提交任务
- 支持cancel一个已经提交的任务
- 支持stop
- 支持高优任务插队

ExecutionQueue和mutex都可以用来在多线程场景中消除竞争. 相比较使用mutex,
使用ExecutionQueue有着如下几个优点:

- 角色划分比较清晰, 概念理解上比较简单, 实现中无需考虑锁带来的问题(比如死锁)
- 能保证任务的执行顺序，mutex的唤醒顺序不能得到严格保证.
- 所有线程各司其职，都能在做有用的事情，不存在等待.
- 在繁忙、卡顿的情况下能更好的批量执行，整体上获得较高的吞吐.

但是缺点也同样明显:

- 一个流程的代码往往散落在多个地方，代码理解和维护成本高。
- 为了提高并发度， 一件事情往往会被拆分到多个ExecutionQueue进行流水线处理，这样会导致在多核之间不停的进行切换，会付出额外的调度以及同步cache的开销, 尤其是竞争的临界区非常小的情况下， 这些开销不能忽略.
- 同时原子的操作多个资源实现会变得复杂, 使用mutex可以同时锁住多个mutex, 用了ExeuctionQueue就需要依赖额外的dispatch queue了。
- 由于所有操作都是单线程的，某个任务运行慢了就会阻塞同一个ExecutionQueue的其他操作。
- 并发控制变得复杂，ExecutionQueue可能会由于缓存的任务过多占用过多的内存。

不考虑性能和复杂度，理论上任何系统都可以只使用mutex或者ExecutionQueue来消除竞争.
但是复杂系统的设计上，建议根据不同的场景灵活决定如何使用这两个工具:

- 如果临界区非常小，竞争又不是很激烈，优先选择使用mutex。
- 需要有序执行，或者无法消除的激烈竞争但是可以通过批量执行来提高吞吐， 可以选择使用ExecutionQueue。




追加日志
1. 调用
```c++
void LogManager::append_to_storage(std::vector<LogEntry*>* to_append, LogId* last_id)
```

传入需要append的Entry vector，返回所有append中最后的一个entry id，并且释放to_append

2. append_to_storage内部调用
```c++   
MemoryLogStorage::append_entries(const std::vector<LogEntry*>& entries)
```

顺序枚举调用函数
```c++
int MemoryLogStorage::append_entry(const LogEntry* input_entry)
```

3. 在append_entry中插入input_entry到 MemoryData中  _log_entry_data（一个队列）并且更新    _last_log_index += 1

4. 