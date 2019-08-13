BAM文件简介
==========
* 用于存储对比数据
* 支持长read和短read，最长支持128Mbp

常见后缀：
-------
* .bam 
* .sam  纯文本格式(The Sequencing Alignment/Map Format)
* .cram 高压缩格式

bwa比对软件 -> output (*.sam) 
文本文件太大了
-> *.bam 二进制的，压缩后size是sam的1/6

基本概念
=======
template
--------
* DNA/RNA序列的一部分，用它进行测序或者由原始数据拼接得到它，作为研究的模版；
* 【参考序列和比对上的序列共同组成的序列为template】

read
----
* 测序仪下机得到的原始数据，这些数据依照测序时间的先后被打上不同标签；双端测序存在read1，read2

segment
-------
* 比对到read的时候的一段连续序列或者被分开几条子序列。一条read可以包含多条segment；

linear alignment
----------------
* 一条read比对到参考序列上，可以存在插入(insert)、缺失(delete)、跳跃(skip)、剪切(clip)，但是方向不变（不能是一部分和正链匹配，另一部分又和负链匹配），sam文件中只占用一行记录；
  * Clip 作为名词讲，有剪下来的东西的意义，
  * 在SAM/BAM 比对文件里面，用于描述那些一条序列上，在序列两端，比对不上的碱基序列
  * 类别：
    * Soft Clip —— 是指虽然比对不到基因组，但是还是存在于SEQ (segment SEQuence)中的序列，此时CIGAR列对应的S(Soft)的符号。直白点说，就是虽然比对不上参考基因组，但是在BAM/SAM文件中的reads上还是存在的序列（并没有被截断扔掉的序列）。
    * Hard Clip —— 就表示比对不上并且不会存在于SAM/BAM文件中的序列, 被截断扔掉了的序列，此时CIGAR列会留下H(Hard)的符号，但是序列的那一列却没有对应的序列了。

chimeric alignment
------------------
* “嵌合比对” 的形成是由于一条测序read比对到基因组上时分别比对到两个不同的区域，而这两个区域基本没有overlap。因此它在sam文件中需要占用多行记录显示。只有第一个记录被称作"representative",其他的都是"supplementary"【Chimeric reads are also called split reads】



FLAGS
=====




@SQ
===
* 参考序列词典，多行，@SQ顺序定义了比对的排序顺序

SN(必选)
-------
* 参考序列的名字，在所有@SQ列中，所有的SN和AN对应的名字必须是唯一的。
* 这个值被用于比对记录中的RNAME和RNEXT列

LN(必选)
-------
* 参考序列的长度

M5,AS,UR,SP,DS
--------------
* M5 参考序列的md5
* AS 基因组装id
* UR 序列的URI，一般给个http地址或者是ftp地址，或者是某个路径
* SP 物种信息
* DS 描述信息

