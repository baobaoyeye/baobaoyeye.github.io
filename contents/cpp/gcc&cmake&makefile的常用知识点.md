gcc/g++
=======
常用选项
------
```-o```表示我们要求输出的可执行文件名。

```-c```选项表示我们只要求编译器输出目标代码，而不必要输出可执行文件。

```-g```选项表示我们要求编译器在编译的时候提供我们以后对程序进行调试的信息。

编译时{加|减}宏定义
----------------
```c++
// file[main.cc]
#include <iostream>
#include <string>

#ifdef PRINT_TEST
std::string test_str = "defined PRINT_TEST macro.";
#else
std::string test_str = "undefine PRINT_TEST macro.";
#endif

int main(){
    std::cout << test_str << std::endl;
    return 0;
}   
```

```shell
# 宏生效
g++ -o test_macro main.cc -DPRINT_TEST

g++ -o test_macro main.cc -D PRINT_TEST

# 宏失效
g++ -o test_macro main.cc -UPRINT_TEST

g++ -o test_macro main.cc -U PRINT_TEST
```

cmake
=====
cmake对大小写不敏感

添加头文件目录
-----------
语法：
```
include_directories([AFTER|BEFORE] [SYSTEM] dir1 [dir2 ...])
```
相当于gcc里面的 -I
举例：
```
include_directories(./ ./include)
```

增加编译选项
----------
```makefile
#判断编译器类型,如果是gcc编译器,则在编译选项中加入c++11支持
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
    message(STATUS "optional:-std=c++11")   
endif(CMAKE_COMPILER_IS_GNUCXX)
```

Makefile
========

格式
---

### 注释 ###
注释行都是用```#```开头的

### 一般格式 ###
```
target：components
TAB rule
```
上面的格式规定：*当commponents在target修改后修改的话，就要去执行rule所指定的命令。*

### 初版Makefile ###

```makefile
main：main.o t1.o t2.o
    g++ -o main main.o t1.o t2.o

main.o：main.c t1.h t2.h
    g++ -c main.c

t1.o：t1.c t1.h
    g++ -c t1.c

# 上面描述的格式可以理解为：
# 如果t2.o修改之后，只要t2.c或者t2.h发生变化，
# 那么就需要执行下一行的规则 g++ -c t2.c
t2.o：t2.c t2.h
    g++ -c t2.c
```

先粗浅的理解一下如下这几个符号

```$@```目标文件

```$^```所有的依赖文件

```$<```第一个依赖文件

### 简化版Makefile ###

```makefile
main：main.o t1.o t2.o
    g++ -o $@ $^

main.o：main.c t1.h t2.h
    g++ -c $<

t1.o：t1.c t1.h
    g++ -c $<

t2.o：t2.c t2.h
    g++ -c $<
```

了解一个新的规则：```.c.o:``` 表示所有的 .o文件都是依赖与相应的.c文件。

### 再简化Makefile ###
```makefile
main：main.o t1.o t2.o
    g++ -o $@ $^

.c.o:
    g++ -c $<
```
清爽许多