

### HoodieSparkSqlWriter做了什么？
#### write
* 做**合法性检查**，从参数`parameters`中拿到 path，tablename，operation
  1. `spark.serializer` 目前只支持 `org.apache.spark.serializer.KryoSerializer`
  2. operator 如果配置了`hoodie.datasource.write.insert.drop.duplicates(默认false)`不允许重复选项为true，并且是upsert那么 operator=insert。否则，如设置
  3. `parameters`列表都有啥？？<details>

     | 选项                                                  | 含义                                                         | 默认值                |
     | ----------------------------------------------------- | ------------------------------------------------------------ | --------------------- |
     | hoodie.datasource.write.insert.drop.duplicates        | 是否允许重复                                                 | false                 |
     | hoodie.datasource.hive_sync.database                  |                                                              | default               |
     | hoodie.insert.shuffle.parallelism                     | suffle的并行度                                               | 10                    |
     | path                                                  | basepath                                                     | file:///tmp/hudi_test |
     | hoodie.datasource.write.precombine.field              | precombine的列                                               | timestamp             |
     | hoodie.datasource.hive_sync.partition_fields          |                                                              |
     | hoodie.datasource.write.payload.class                 | org.apache.hudi.common.model.OverwriteWithLatestAvroPayload, |
     | hoodie.datasource.hive_sync.partition_extractor_class | org.apache.hudi.hive.SlashEncodedDayPartitionValueExtractor, |
     | hoodie.delete.shuffle.parallelism                     | 10,                                                          |
     | hoodie.datasource.write.streaming.retry.interval.ms   | 2000,                                                        |
     | hoodie.datasource.hive_sync.table                     | unknown,                                                     |
     | hoodie.datasource.write.streaming.ignore.failed.batch | true,                                                        |
     | hoodie.datasource.write.operation                     | upsert,                                                      |
     | hoodie.datasource.hive_sync.enable                    | false,                                                       |
     | hoodie.datasource.write.recordkey.field               | key,                                                         |
     | hoodie.table.name                                     | hudi_test,                                                   |
     | hoodie.datasource.hive_sync.jdbcurl                   | jdbc:hive2://localhost:10000,                                |
     | hoodie.datasource.write.table.type                    | COPY_ON_WRITE,                                               |
     | hoodie.datasource.write.hive_style_partitioning       | false,                                                       |
     | hoodie.datasource.hive_sync.username                  | hive,                                                        |
     | hoodie.datasource.write.streaming.retry.count         | 3,                                                           |
     | hoodie.datasource.hive_sync.password                  | hive,                                                        |
     | hoodie.datasource.write.keygenerator.class            | org.apache.hudi.keygen.SimpleKeyGenerator,                   |
     | hoodie.upsert.shuffle.parallelism                     | 10,                                                          |
     | hoodie.datasource.write.partitionpath.field           | dt,                                                          |
     | hoodie.datasource.write.commitmeta.key.prefix         | _,                                                           |
     | hoodie.bulkinsert.shuffle.parallelism                 | 10                                                           |
     </details>
* 构造**JavaSparkContext**，根据scala 中的`sqlContext.sparkContext`
* 得到**Instant时间**，通过调用`HoodieActiveTimeline.createNewInstantTime()` 本地的单调增时间，理解成Event Time 格式是`yyyyMMddHHmmss` 如：`20200416031225`
* 拿到**文件系统fs**
* 判断**元信息目录是否存在** `{basepath}/.hoodie`
* 构造**avro的GenericRecord RDD**, 通过`AvroConversionUtils.createRdd`，参数:`structName={tablename}_record, nameSpace=Hoodie.{tablename}`
* 构造**Pair<JavaRDD\<WriteStatus>, HoodieWriteClient>**
  * case: operator != **Delete** 
    * 将**StructType转换成Avro的Schema**，通过 `AvroConversionUtils.convertStructTypeToAvroSchema`
    * 将**AvroSchema注册到SparkConf**，通过 `registerAvroSchemas`
    
    * 将`GenericRecord RDD` 转换成 `HoodieRecord JavaRDD` （完成avro到HoodieRecord的转换）
    * 检查spark.write...mode(xxx)参数中的mode决定是否要抛出异常或者要删除历史目录。Overwrite模式需要清除历史目录
    * 调用 `HoodieTableMetaClient.initTableType`，继而调用`initTableAndGetMetaClient` 初始化hoodie表的元信息<details>

      * 按需创建一系列目录 basepath, metapath, archivelogpath, tmpdir, auxdir
      * 调用HoodieTableConfig.createHoodieProperties 把hoodie表的属性写到元信息中
        * 包括：
          * 表名
          * 表类型（默认是 `COW` ）
          * payloadclass名（表类型是 `MOR` 的时候才需要这个属性）
          * archivelogpath，[干啥的？？]()
          * timelineLayoutVersion（不同版本的时间轴不同用一个版本区分）
        * 默认:
          ```java
          #Properties saved on Wed Apr 15 19:00:25 CST 2020
          #Wed Apr 15 19:00:25 CST 2020
          hoodie.table.name=hudi_test
          hoodie.archivelog.folder=archived
          hoodie.table.type=COPY_ON_WRITE
          hoodie.timeline.layout.version=1
          ```
      * 创建 HoodieTableMetaClient 
        * 根据basepath找到metapath，构造HoodieTableConfig，
          * 实际上就是从 basepath/.hoodie/hoodie.properties 读配置（如果是新的表怎么办？？）
          * 写 hoodie.compaction.payload.class = payloadClassName
          * 写注释 Properties saved on = 更新的时间

    </details>

    * 创建**HoodieWriteClient**，通过调用 `DataSourceUtils.createHoodieClient` 传入AvroSchema的字符串形式
    * 按需决定**是否过滤重复记录**，如果`hoodie.datasource.write.insert.drop.duplicates=true`会过滤
    * 开始**提交**, 
  * case: operator == **Delete** 
    * 保证SaveMode一定是Append
    * 将`GenericRecord RDD` 转换成 `HoodieRecord JavaRDD` （因为是delete，所以只需要key即可）
    * 保证表元信息存在
    * 创建**HoodieWriteClient**,通过调用 `DataSourceUtils.createHoodieClient` 传入空的schema
* **检查错误提交写**,通过调用`checkWriteStatus`

### createHoodieClient做了什么？

### commit的般流程是什么？

* 看HoodieWriteClient.rollbackPending要不要把正在等待的action回滚掉？
  * 需要回滚构造一个 HoodieTable