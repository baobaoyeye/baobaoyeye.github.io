HoodieArchivedTimeline
======================

干什么的？
-------
* 用来描述某个Table中已经归档的时间轴（什么样的时间点被放到归档时间轴中？？？）
* 维护一坨时间点（instants）
  * Q:从哪里拿来时间点？ 
    > A：文件系统中Archived目录下面的Log文件
  * Q:其中的HoodieInstant都包括什么类型的？
    > A：不能是inflight的

有什么特性
--------
* 可以给定区间过滤
* 维护的instants不需要详细信息（details）
* 拿到的Instants列表实际上是乱序的：单文件中由旧到新，文件间由新到旧

文件名格式
--------
`.commits_.archive.[0-9]+`

比较器
----
文件间按照版本**逆序**比较,这部分工作交由`ArchiveFileVersionComparator`处理，也就是越新的文件在列表中越靠前

过滤器
-----
只需要知道一点：给定区间是左开右闭 (start, end]


核心工作
------
**从文件系统的特定目录加载Instant时间列表——loadInstants**
1. 从特定目录下读所有的符合上述格式的文件FileStatus信息
2. 按照上述的比较器，重排序FileStatus列表，越新越靠前
3. 迭代每个FileStatus
   1. 按照ArvoSchema构造HoodieLogFileReader
   2. 依次读每个HoodieLogBlock 中的Records（这里面的block特指 HoodieAvroDataBlock）
   3. 依次从Records中读取到 commitTime 和 actionType 两个字段, **构造出HoodieInstant** 如果需要详细字段，会以 Map<commitTime, details >维护
   4. 如果存在过滤器，就按照特定区间过滤一波
   5. 最后针对有filter的情况优化了一波，大致意思是：如果某个文件过滤之后发现没有找到任何instant那之前的文件也就不需要再找了。


惊奇的发现竟然是读Log，难道archive的log（log就是avro行式的二进制格式）hudi中把avro的schema avsc文件，定义到hudi-common下的avro目录中。

