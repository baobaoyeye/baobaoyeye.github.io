* [概述](#概述)
   * [关于文档TOC的生成](#关于文档toc的生成)
   * [关于标题的建议](#关于标题的建议)
* [Title1](#title1)
   * [Title 2](#title-2)
      * [Title 3](#title-3)
         * [Title 4](#title-4)
   * [建议统一的写法](#建议统一的写法)

概述
===
该文档描述一些开源前文档编写的格式和注意事项，为了避免后面维护的成本提高，建议大家统一格式。

关于文档TOC的生成
---------
每页文档开头增加索引TOC，方便阅读和review，目前github不支持`[TOC]`标签直接生成。  
在docs/tools/目录下有个脚本，可以对当前文档直接生成toc到标准输出。贴到自己文档开头即可, 每次变更结构记得重新生成。命令如下：
```
cat your_doc.md | gh-md-toc -
```
该工具开源前*删掉*，后续可以内部维护。

关于标题的建议
------------
一级用 ==== 方式，二级用 ---- 方式，三级及三级以上用 ### 方式，具体如下：
```
Title1
======
Title 2
-------
### Title 3
#### Title 4
```
Title1
======
Title 2
-------
### Title 3
#### Title 4

1. ordered list item 1
2. ordered list item 2
3. ordered list item 3  


* unordered list item 1
* unordered list item 2
* unordered list item 3

建议统一的写法
-----------
 **方便以后因目录变更引起的链接等失效，可以批量处理**

* 加外部链接  
```
[outer link](https://google.com)  
[docs inner link](../README.md) 采用相对路径
```
[outer link](https://google.com)  
[docs inner link](../README.md) 采用相对路径

* 加注脚或引用  
github 目前还不支持注脚，建议大家使用如下方式做注脚
```
正文某处 [引用1][1] 正文又一处 [引用2][2] 正文再一处 [引用3][3]
引用实际链接在文末整体给出。
```
正文某处 [引用1][1] 正文又一处 [引用2][2] 正文再一处 [引用3][3].  

* 直接引用文字
```
> 我是引用文字
```
> 我是引用文字  

* 加图片  
```
![img](../images/demo.png) 采用相对路径
```
![img](../images/demo.png) 采用相对路径

* 贴代码c++ python java ...
``` c++
/**
 * your c++ codes or commands
 **/
class Test {
public:
  void MemberMethod1()
private:
  int member_v1_;
};
```
* 关于文件头的copyright  
大家可以直接复制这段到对应文件的头部即可  

```
// Copyright (c) 2018-present, Meituan Dianping. Inc.
//   Author: xxx
//
// Based on (c) 2007-2010 Taobao Inc.
//   Author: xxx
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; under version 2 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

your codes

```

如果文件不是冲taobao衍生过来的，可以去掉其中的

```
// Based on (c) 2007-2010 Taobao Inc.
//   Author: xxx
//
```


[1]: http://google.com/        "引用1"
[2]: http://search.yahoo.com/  "引用2"
[3]: http://search.msn.com/    "引用3"
