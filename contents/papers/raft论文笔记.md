## 寻找一个易懂的一致性算法

### 摘要
raft是一个管理复制日志的一致性算法。它提供等同于multi-paxos的结果。并且它和paxos一样高效。但是它和paxos结构不同。这使得raft相比于paxos更容易被理解
并且可以为构建实际系统提供一个更好的基础。为了更好的被理解，raft区分一致性过程中的关键元素，比如leader选举，日志复制，安全，
并且它执行一个更强的一致来减少必须被考虑的状态的数量。带来的结果就是raft比paxos更易学。  
raft也包含一个修改集群成员的新机制，使用重叠的大多数来保证安全。

### 1、介绍
一致性算法允许一组机器以一个一致的group工作，能够允许部分成员失败。因为这个它成为大规模软件系统构建中的关键规则。paxos一直主宰一致性算法，
很多算法都是基于paxos或者他的衍生。但是paxos太难了，paxos需要经过复杂的修改才能适用真实系统。  
作者设计出一个新的一致性算法，易懂易建。主要目标就是易懂。重要的不是这个算法可以work，而是为什么可以work。
通过分解和状态空间的合并来使得raft更易懂。
* 分解：leader选举、日志复制、安全
* 合并状态空间：不确定程度、server之间不一致的方式。

raft和存在的一致性算法有点相似，但他有些新特性：
* 强leader：使用强leader，比如log只能从leader流向其他的servers。这个使它易于理解并且方便管理日志复制。
* leader选举：使用随机计时器来选举leader。只在心跳中增加少量的成本但是高效容易的解决了冲突。
* 成员变更：使用joint consensus（共享一致：变更过程中两个不同配置重叠的大多数）方法解决成员变更，这个使集群可以在配置变更过程可以持续完成正常操作。

### 2、复制状态机
在一个servers集上状态机为相同的状态计算相同的副本，即使有些机器down掉也可以持续操作。
复制状态机用来解决一系列分布式系统中失败恢复的问题。
复制状态机典型实现是复制日志，如图一：
每个服务存储一个包含一系列命令的日志，他们的状态机顺序的执行。每个日志在相同顺序上包含相同的命令，因此每个状态机处理相同命令序列。
因为状态机是确定的，因此计算的状态和输出都是相同。

保证复制日志的一致性是一致性算法的工作。服务上的一致性模块从客户端接收命令并且把他们加入到日志中。一致性模块和其他服务上的一致性模块通信，
来确认每个日志最终以相同的顺序包含相同的请求，即使有些服务失败。

一旦命令被正确的复制，每个服务的状态机以log顺序处理他们，并且输出返回给客户端。

这样服务就有一个简单，高可用的状态机。

真实系统中一致性算法通常有下面这些属性：
* 在所有非拜占庭条件下都是安全的（永远不会返回错误的结果）包括：网络延迟，分区，丢包、重复和重排序等。
* 只有有服务的大多数能够彼此通信并和客户端通信，服务就可用的。举例一个五台机器的集群能够容忍2台失败。服务停止被认为是失败；他们可能恢复并重新加入到集群中。
* 不依赖定时确认日志的一致性：错误的时钟和极端的消息延时，在最坏的情况下会引起可用性问题。
* 通常的case，一个命令能够完成 集群中的多数派响应在一个单轮的RPC；少数慢服务不能影响整个系统的性能。

### 3、Paxos错在哪？
等研究完paxos再把这部分补充上去
### 4、为易懂而设计
raft有一系列的设计目标
* 明显的减少开始者设计的工作量；
* 在所有条件下都是安全的，在典型的操作条件下可用；
* 通用的操作需要高效
* 最重要的是易懂
这个算法必须可以凭直觉就能开发出来，是的系统构建者在真实世界的实现中可以扩展。
当有多个备选方案的时候需要考虑读者易懂和实现上更巧妙。
有两个方法：
* 问题分解，把问题切成不同的小块，每块都容易解决相对易懂，比如切分leader选举，日支复制，安全和成员变更。
* 简化状态空间，减少需要考虑的状态数量，是系统更清晰并且干掉可能不确定的地方。
  日志不允许有洞，并且限制了日志变得彼此不一致的方式。尽管很多厂家我们尝试消除不确定
  有些场景下不确定实际上提高了易懂。特别的，随机方法引入不确定，
  但是这些方法通过类似的方式处理所有可能的选择，来归并状态机。我们使用随机的方式来简化
  leader选举算法。

### 5、Raft一致性算法
raft通过选择一个强主，然后提供给leader管理复制log的完整职责。
leader从客户端接收日志entry，然后复制日志到其他的server，并且告诉他们什么时候应用日志到他们的状态机中是安全的。
有一个leader可以简化日志复制的管理。
比如，leader可以在不需要和其他server商议的情况下就决定一个entry放到日志中的哪个位置。并且数据流简单的从主流向其他server。
leader可以失败或者和其他server断连，如果是那样的话会选一个新的leader出来。

raft分解一致性问题到3个相对独立的子问题
* leader 选举
  当一个已存在的leader失败了，一个新的leader必须被选出来。
* 日志复制
  leader从client处接受日志，并在集群中复制，强制其他日志同意leade自己的。
* 安全
  raft中最关键的安全属性是图3中的状态机安全属性。如果任何一个server已经应用特定的log entry到他的状态机。
  那么没有任何一个server可以在相同的log index 处应用其他命令。raft实现这个安全属性，依赖选举机制中附加的限制条件。

#### 5.1、raft基础
一个raft集群包含数个server，5个是一个典型的数量，可以容忍两个服务失败。
在任何一个时间上，一个server都有3个状态：leader、follower、candidata （候选人）
正常情况下，会有一个leader其他全部是follower。
follower是被动的，follower除了简单答复leader和candidate的请求以外，不处理任何请求。
leader处理所有client的所有请求，如果client的请求打到follower上，follower负责把请求重定向到leader，candidate负责选出一个新的leader
![roles](../../images/raft/roles.png)

raft把时间切成一段一段的，每段叫做一个term，每个trem开始于一个新的选举:
* 成功的选举（一个或多个candidate企图成为leader,没出现平票），leader管理整个集群，直到term结束
* 失败的选举（出现平票），这时候没有选出leader，term就结束了   
raft确定一个term中最多一个leader
![term](../../images/raft/term.png)
term是连续的整数编号的。
不同的server观察到的term间转换可能发生在不同的时时间，在某些场景下有个server有可能观察不到一个选举过程甚至整个term（s）。
terms在raft中担任逻辑时钟的角色，并且terms允许servers观察到旧的leaders。
每一个server保存current_term_no，term号随时间流逝单调增。
当server间通信的时候current_term_no可能被替换，如果一个server的current_term_no比其他的server的小，那么他会更新current_term_no到一个更大的值。如果candidate或者leader发现自己的term已经超时了，它立即把自己的状态变成follower,如果一个server收到一个带着旧的term号的请求，它拒绝这个请求。
raft服务间通过rpc进行通信，一个基本的一致性算法，需要两种类型的rpc。
* RequestVote rpc，由candidate在选举的过程中初始化
* AppendEntry rpc， 由leader初始化，在复制日志过程，并且会提供心跳状态。
* 第三种类型的rpc，用于server之间传输快照

server会重试rpc，如果他们一段时间没有收到回复的话，并且为了高性能他们会并行发起rpc。
#### 5.2、leader选举
raft使用心跳机制触发leader选举，当服务启动的时候他们都是follower，只要有从candidate或者leader发起的有效的rpc被server收到，它就一直停留在follower状态。为了维护权威leader周期性的发送心跳,通过不带log entry的 AppendEntry rpc完成心跳传输。如果一个follower在一段时间内（election timeout）没有接收到通信，然后它会假设没有可用的leader存在，开始一轮新的选举。开始一个选举，follower会增加它的current_term_no，并且改变自己的状态为candidate。它先给自己投一票，然后并发的发送RequestVote请求给集群中的其他server，一个candidate会在下面3个事情有一个发生的时候退出这个状态
* 它赢得选举了（赢了）
* 另一个candidate确认自己是leader（败了）
* 过了一段时间但是没有leader被选出来（平票）

如果一个candidate在同一个term中赢得了集群中大多数机器的选票，我们认为这个candidate赢得了选举。在给定的term中某个机器最多只能给一个candidate投票，基于先到先服务的原则（5.4还有个附加条件）
大多数规则保证在特定的term中最多有一个candidate能够赢得选举，当一个candidate赢得了选举，他会成为leader。随后它会发心跳给所有其他的server，确认自己的权威，并且阻止新的选举。

在等选票的过程中，candidate可能会收到一个来自于声称自己是leader的其他server发送来的AppendEntry RPC请求，
* 如果这个leader的term号大于等于这个candidate的term，这个candidate会承认leader的合法性并且返回follower状态。
* 如果这个leader的term号小于这个candidate的term，那么这个candidate拒绝这个rpc并且继续candidate状态。

第三种可能就是这个candidate没赢也没输：如果同一个时间有多个candidate产生，票会被切分。所有candidate都没有得到大多数选票。当这种情况发生，每个candidate会超时并且开始一个新的选举增加它自己的term号并且初始化另一轮RequestVote rpc。如果没有附加的限制分票（split vote）行为可能会持续进行下去。

raft通过随机超时保证极少会出现选举分票，即使出现也可以很快恢复。
为了阻止最初的分票，选举超时随机从一个小区间选（150-300ms）。通过这个方法可以分散server的超时时长，大多数的case中只有一个server会超时，它赢得选举并且在其他server都没超时前发送心跳。
相同的机制去处理分票，一个candidate在发起一个新的选举时会重置一个新的随机超时时间。并且在开始下一次选举前它等待这个超时时间的到来。这会减少在新一轮选举中分票的可能性。

对于选举，作者也想过用排序的方式，每个candidate分的唯一个一个位置，选举过程如果位置靠前的candidate会更容易当选，但是这也会出现可用性问题，当出现排序靠前的candidate失败的时候，排序靠后的candidate需要等待一个超时时间然后发起一个新的选举。他们觉得这些方法都有各种各样的问题，最后选了易懂易实现的随机超时的方法来选leader。

#### 5.3、日志复制
当leader被选出来以后，他开始服务于client，每个client的请求包含一个要被复制状态机执行的命令。leader 追加这个命令作为它自己log的一个新entry，然后并行的发AppendEntry RPC到其他的server上，当这个entry被安全的复制了，leader应用这个entry到他的状态机中并且返回执行结果给client。如果followers crash或者比较卡，又或者丢包了，leader会无限期的发AppendEntry rpc（尽管这时候它已经相应了client），直到所有的follower最终保存了所有的log entry。

日志的组织如下图，每一个Log由顺序编号的entry组成，每个entry中包含一个创建它时候的term对应的term号和一个状态机执行的命令。如果一个entry被应用到状态机中会被认为是commited。
log entry中的term号用来发现log间的冲突。每个log entry包含一个整数索引，表示它在log中的位置。
![log](../../images/raft/log.png)

raft保证所有commited entry都是持久的。并且最终会被所有可用的状态机执行。一个log entry在创建它的那个leader复制它到集群中的大多数server以后才会被提交。如上图中的entry[7],也会提交当前leader log中所有之前的entry，包括前面leader遗留下来的entries

leader保有着所有commited entry中最大的index，并且在后面的AppendEntry rpc（包括心跳）过程中会携带这个index，因此其他的server最终都会发现。当一个follower发现一个log entry已经被提交了，它会按照log index的顺序依次把对应entry的命令应用到状态机。

raft log机制使其更安全，这个不同于一般的server log。

日志一致属性：
条件：*如果*在不同log中的两个entry，他们具有相同的index和term
* *那么*这**两个entry**存储相同的命令
* *那么*在**不同的log**间所有前面的entry都是一样的
第一个属性依赖一个事实：leader在一个term中对于一个给定的log index最多只能创建一个entry，并且entry永远也不会修改他们在log中的位置。
第二个属性通过AppendEntry做简单的一致性检查就可以保证。当发送一个AppendEntry RPC的时候，leader会发送一个即将会出现的entry的index和term给其他的server，
#### 5.4、安全
##### 5.4.1、选举限制
##### 5.4.2、来自先前term的提交
##### 5.4.3、安全争论
#### 5.5、follower和？？？？
#### 5.6、计时和可用性

### 6、集群成员变更

### 7、日志整理

### 8、客户端交互

### 9、实现和评估
#### 9.1、易懂
#### 9.2、正确性
#### 9.3、性能

### 10、相关工作

### 11、结论

### 12、感谢
