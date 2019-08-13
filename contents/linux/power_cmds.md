硬件命令
=======

查看所有硬件
----------
```bash
$ dmidecode | more
```

查看CPU信息(0)
-------------
```bash
$ cat /proc/cpuinfo |more  # 全部信息
$ cat /proc/cpuinfo | grep 'physical id' | sort | uniq | wc -l  # 多少个物理CPU芯片
$ cat /proc/cpuinfo | grep 'cpu cores' | sort | uniq  # 每个物理CPU芯片上core数目
$ cat /proc/cpuinfo | grep 'processor' | wc -l  # 逻辑cpu数目
```

* 上面的**core数目**不一定完全等于**逻辑cpu数目**，
  * 当为开启超线程的时候二者相等，
  * 否则逻辑cpu数目core数目的二倍

通过```cat /proc/cpuinfo```看到的cpu信息举例如下
```bash
...
processor	: 31           # 当前逻辑cpu的序号，从0开始
vendor_id	: GenuineIntel # 制造商
cpu family	: 6            # 产品系列代号
model		: 79           # 系列内的代号
model name	: Intel(R) Xeon(R) CPU E5-2620 v4 @ 2.10GHz
stepping	: 1
cpu MHz		: 2101.000     # 主频
cache size	: 20480 KB     # CPU最高级缓存的大小
physical id	: 1            # 当前逻辑cpu所在物理cpu id
siblings	: 16           # 当前物理cpu上有多少个逻辑cpu（包括超线程）
core id		: 7            # 当前core所在物理cpu中的序号，从0开始
cpu cores	: 8            # 当前物理cpu有多少个core
apicid		: 31
initial apicid	: 31
fpu		: yes              # 是否有浮点运算单元
fpu_exception	: yes      # 是否支持浮点计算异常
cpuid level	: 20           # 执行cpuid指令前，eax寄存器中的值，
wp		: yes              # 内核态是否支持写保护
flags		: fpu...rtm    # 当前CPU支持的功能

bogomips	: 4189.99
clflush size	: 64       # 每次cache line 刷多少数据到主存 （单位：字节）
cache_alignment	: 64       # cache line 以多少字节对齐 
address sizes	: 46 bits physical, 48 bits virtual
power management:
```

查看CPU信息(1)
------------
* 可以使用```lscpu```来查看cpu的信息，比较简洁明了，命令及结果如下：
```
$ lscpu
Architecture:          x86_64
CPU op-mode(s):        32-bit, 64-bit
Byte Order:            Little Endian       # 大小端
CPU(s):                32
On-line CPU(s) list:   0-31
Thread(s) per core:    2
Core(s) per socket:    8
Socket(s):             2
NUMA node(s):          2
Vendor ID:             GenuineIntel
CPU family:            6
Model:                 79
Model name:            Intel(R) Xeon(R) CPU E5-2620 v4 @ 2.10GHz
Stepping:              1
CPU MHz:               2101.000
BogoMIPS:              4189.99
Virtualization:        VT-x
L1d cache:             32K                 # L1数据cache
L1i cache:             32K                 # L1指令cache
L2 cache:              256K
L3 cache:              20480K
NUMA node0 CPU(s):     0-7,16-23
NUMA node1 CPU(s):     8-15,24-31
```

RPM工具使用
=========

查看以安装软件
-----------
```
# rpm -qa | grep -i openssl


openssl-devel-1.0.1e-57.el6.x86_64
openssl-1.0.1e-57.el6.x86_64
```

查看安装目录
----------
```
# rpm -ql openssl-1.0.1e-57.el6.x86_64
```
检查安装包完整性
-------------
```
# rpm -V openssl-1.0.1e-57.el6.x86_64

S.5....T.  d /usr/share/doc/openssl-1.0.1e/FAQ
```
如果没有任何变更，则不会出现任何输出，只要输出即认为是不完整的

lib*.so相关
==========
打印出当前缓存所保存的所有库的名字
```
# ldconfig -p | grep xxxx
```