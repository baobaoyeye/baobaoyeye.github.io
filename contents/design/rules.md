一些面向对象的观点
===============

纲领
---

### SOLID
* S
* O
* **L (Liskov Substitution principle)里氏替换原则** 
  
  原文（来源：[维基百科](https://zh.wikipedia.org/wiki/%E9%87%8C%E6%B0%8F%E6%9B%BF%E6%8D%A2%E5%8E%9F%E5%88%99)）
  > Let q(x) be a property provable about objects x of type T. Then q(y) should be true for objects y of type S where S is a subtype of T.

  翻译
  >派生类（子类）对象可以在程序中代替其基类（超类）对象
* I
* D

积累
---
* 让每个类或成员尽可能地不可访问。