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

bam的格式
========
