## zmq
&emsp;&emsp;第一次发现这个是在看Anna的cmake。zmq(http://zeromq.org/ )是一个高性能异步消息库，致力于分布式和并行应用。
它提供一个消息队列，但又不像面向消息的中间件，它可以不需要一个专职的<code>消息代理</code>，
它的接口设计有点类似<code>Berkeley sockets——与Posix sockets的本质相同</code>
   提供了几种模式：
   * request-reply     请求响应 RPC or 任务分发模式 对应技术：服务总线
   * publish-subscribe 发布订阅模式，数据分发模式 对应技术：数据分发树
   * push-pull (pipeline) 并行的任务分发收集模式， 对应技术：并行管道
