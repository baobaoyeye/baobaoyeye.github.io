RDD
===

* 一个弹性分布式数据集。以并行方式处理的，跨多个集群中节点的分割元素组成的集合。RDD可以从节点故障中自动恢复。
* 可以由以下几种方式创建：
1. HDFS（兼容文件系统）中的文件
2. 驱动程序中的一个Scala集合转换得来

共享变量
=======

* 广播变量：能够使用可以被缓存在所有节点的内存中。
* 累加：只能被”加“，类似计数器和求和

DataSource API
==============
[介绍API V1和V2的区别](http://shzhangji.com/cnblogs/2018/12/09/spark-datasource-api-v2/)
[使用java构建Spark的数据源](https://www.slideshare.net/databricks/extending-sparks-ingestion-build-your-own-java-data-source-with-jean-georges-perrin)
[Spark Sql-自定义数据源](https://nightpxy.github.io/2018/07/11/Sql-%E8%87%AA%E5%AE%9A%E4%B9%89%E6%95%B0%E6%8D%AE%E6%BA%90/)

BaseRelation
------------

一个数据源的抽象描述，核心就是schema。


* 实现如下`CreatableRelationProvider`接口中的`createRelation`可以完成将某个DataFrame写到一个目的地。
```scala
trait CreatableRelationProvider {
  /**
   * Saves a DataFrame to a destination (using data source-specific parameters)
   *
   * @param sqlContext SQLContext
   * @param mode specifies what happens when the destination already exists
   * @param parameters data source-specific parameters
   * @param data DataFrame to save (i.e. the rows after executing the query)
   * @return Relation with a known schema
   *
   * @since 1.3.0
   */
  def createRelation(
      sqlContext: SQLContext,
      mode: SaveMode,
      parameters: Map[String, String],
      data: DataFrame): BaseRelation
}
```
与其对应的操作，通常类似：

```scala
inputDataFrame.write.
    format("org.apache.hudi"). // 某个实现CreatableRelationProvider的DataSource 这里是 org.apache.hudi.DefaultSource
    options(someOptions).      // 部分 parameters: Map[String, String], someOptions是一些kv，对应 parameters[k] = v
    mode("Overwrite").         // 对应 mode: SaveMode,
    save(somePath)             // parameters[path]
```
