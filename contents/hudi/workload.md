HoodieRecordLocation
--------------------
* 文件Id
* instant时间

WorkloadStat
------------
* insert数目
* update数目
* <更新的文件id, <更新instant时间，update数目>>

WorkloadProfile
---------------
* TaggedRecords Tagged是什么意思？？
* 全局的WorkloadStat
* Map<partitionPath, WorkloadStat>

**如何profile的**
* 统计不同的partitionPath下面records的数目
* 枚举每个partitionPath的数据，构造这个partition的WorkloadStat信息，并累加到全局的WorkloadStat