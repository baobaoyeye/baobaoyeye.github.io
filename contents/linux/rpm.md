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
