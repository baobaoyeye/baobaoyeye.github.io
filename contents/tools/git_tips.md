Git常用
======

告诉git你是谁
-----------

初始的git环境push前需要配置你的user.name和user.email

```bash
$ git config --global user.name "Your Name Here"
$ git config --global user.email "Your Email Here"
```

配置SSH公私钥
-----------

**让你摆脱每次输入密码的窘境**

```bash
$ ssh-keygen
Generating public/private rsa key pair.
Enter file in which to save the key (/home/{your_home}/.ssh/id_rsa):
Created directory '/home/{your_home}/.ssh'.
Enter passphrase (empty for no passphrase):
Enter same passphrase again:
Your identification has been saved in /home/{your_home}/.ssh/id_rsa.
Your public key has been saved in /home/{your_home}/.ssh/id_rsa.pub.
The key fingerprint is:
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx xx@xx.xx
```
生成公私钥，中间提示输入密码，直接回车就好，默认会生成到```~/.ssh/```

```bash
$ cat ~/.ssh/id_rsa.pub
```
将生成的公钥输出拷贝到git服务器上，一般的git服务提供对应add public key的接口

同步fork仓库
------------

**让你Pull Request更顺畅**

设置本地的upstream到中心的仓库URL
```bash
git remote add upstream URL
```
然后利用fetch和merge合并upstream的master分支,此时本地的master分支就更新至upstream的master版本
```bash
git fetch upstream
git merge upstream/master
```
利用push将本地分支覆盖到git远程分支上
```bash
git push origin master:master
```



cherry-pick
-----------

**保证整洁的commit log**

### Q1:

* 在本地 b1 分支上做了一个commit {X0}，把其放到本地b2分支

### A1:

* 基础命令

```shell
$ git cherry-pick <commit id>
```

* 举例

```shell
$ git checkout b2
$ git cherry-pick {X0}     # 这个 {X0} 号码，位于：

$ git log 
commit {X0}
Author: Somebody <example@a.b>
Date: sometime
```

* 如果顺利，就会正常提交。结果：

```shell
Finished one cherry-pick.
# On branch b2
# Your branch is ahead of 'origin/b2' by {x0} commits.
```

* 如果在cherry-pick 的过程中出现了冲突

```shell
Automatic cherry-pick failed.  After resolving the conflicts,
mark the corrected paths with 'git add <paths>' or 'git rm <paths>'
and commit the result with: 

        git commit -c {X1}
```

* 手工解决冲突

```shell
$ git status    # 看哪些文件出现冲突

both modified:      dir/foo.c 

$ vim dir/foo.c  # 手动解决它。 
$ git add dir/foo.c
$ git commit -c {X1}
``` 