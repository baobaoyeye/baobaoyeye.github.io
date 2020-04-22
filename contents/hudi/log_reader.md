如果想把log文件中的avro数据加载到内存，需要Reader

一个Log Reader 是什么？ 其实就是一个迭代器，
迭代器主要就那么几个接口：

```java
hasNext();

hasPrev();

next();

prev();

```

Log Reader在Hudi中有两种实现:

* 一种是`HoodieLogFileReader` 的作用目标是 `HoodieLogFile` 也就是只负责读一个Log文件
    * 读一个Log需要什么？
        > 需要告知：文件系统是什么，读哪个文件，按照什么Schema读，每次按照多大的buffer读
        > hudi 还有一些自己的特性：
            1. readBlockLazily 不读block内容只读一些元信息,在需要的时候才读内容
            2. reverseReader 是否从后往前读block？
* 一种是`HoodieLogFormatReader` 目的是读一坨LogFile用的里面会维护，当前在用的`HoodieLogFileReader` 所以说FormatReader是一个更高级的接口
  * Q： 核心是什么？ A: 当前的 HoodieLogFileReader 是谁，以及如何找到它（hasNext怎么写）
  * 目前还不能逆向的读，上来就指向了第0个文件的Reader，随即从列表中删掉第0个文件，
  * 需要关注的是：如果用lazily方式读Block则，需要保持读过的reader依然处于open状态，直到最后整体关闭的时候才关闭，否则读完一个Log对应的reader即可close掉


要迭代什么呢？ HoodieLogBlock

Block 开始的地方有个MAGIC
每次hasNext的时候目的就是看看有没有Magic在
每次next的时候就是readBlock
1. 读size （Long） 说明block并不是定长的，有长有短？？
2. 读version （Int） 看了一下不同版本的日志格式并不相同，具体什么版本的Log格式定义在 `HoodieLogFormatVersion` 
   目前看有两种version：
   
   | 项目           | default(0) | 1    |
   | -------------- | ---------- | ---- |
   | MagicHeader    | true       | true |
   | Content        | true       | true |
   | ContentLength  | true       | true |
   | Ordinal        | true       | true |
   | Header         | false      | true |
   | Footer         | false      | true |
   | LogBlockLength | false      | true |

    version = 1
   | 长度 | Long       | Int  | Int        | [<Int, Int, byte[]>,...] | Long         | byte[]  | [<Int, Int, byte[]>,...]   | Long       |
   | ---- | ---------- | ---- | ---------- | ------------------------ | ------------ | ------- | -------------------------- | ---------- |
   | 字段 | block size | type | meta count | meta <index, size, data> | content size | content | footer <index, size, data> | block size |
   | 解释 | Block长度  | 类型 | 元数据数目 | 元数据列表               | 内容长度     | 内容    | footer                     | Block长度  |

   Block类型：COMMAND, DELETE, CORRUPT, AVRO_DATA 
   HoodieLogBlock是一个抽象类，下面有4个子类，对应上面的Block类型，分别描述不同类型的Block

   | 类型                | 内容                                                                      | 描述                                           |
   | ------------------- | ------------------------------------------------------------------------- | ---------------------------------------------- |
   | HoodieAvroDataBlock | 1. version<br>2. records count <br>3. [<record size, record content>,...] | 用于保存Client发过来需要insert或者upsert的数据 |
   | HoodieDeleteBlock   | 1. version<br>2. length <br>3. Kryo Serialized keys[]                     | 用于保存Client发过来需要delete的keys           |
   | HoodieCommandBlock  | null                                                                      | Content中没有内容，内容在Meta中维护            |
   | HoodieCorruptBlock  | don't care                                                                | 错误的block                                    |
    

   元数据类型：INSTANT_TIME, TARGET_INSTANT_TIME, SCHEMA, COMMAND_BLOCK_TYPE
   Footer和MetaData格式一样
   一个Block有两个长度，开始一个， Q:结尾一个目的是干啥？？ A:逆向迭代 （这里思路不错哦）
   
3. 


这时候需要关注一下 `HoodieArchivedMetaEntry`