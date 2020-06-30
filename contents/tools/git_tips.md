Git常用
======

告诉git你是谁
-----------

初始的git环境push前需要配置你的user.name和user.email

```bash
$ git config --global user.name "Your Name Here"
$ git config --global user.email "Your Email Here"
```

设置一个好用的编辑器
-----------------

```bash
$ git config --global core.editor vim
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

为不同的git仓库配置独立的SSH公私密钥
-------------------------------

**通常你公司可能有一个gitlab之类的仓库server，同时你又是一个热衷开源的年轻小伙，github当然必不可少**

```bash
$ ssh-keygen -t rsa -C 'your_github_mail_address' -f ~/.ssh/github-id-rsa
$ ssh-keygen -t rsa -C 'your_company_mail_address' -f ~/.ssh/gitlib-id-rsa
```

编辑你的 ~/.ssh/config 文件，没有的话创建一个

```
# gitlab
Host your_company_hostname
    HostName your_company_hostname
    User your_name
    PreferredAuthentications publickey
    IdentityFile ~/.ssh/gitlab-id-rsa
# github
Host github.com
    HostName github.com
    User your_name
    PreferredAuthentications publickey
    IdentityFile ~/.ssh/github-id-rsa
```

将公钥复制到对应的仓库服务上即可

```bash
$ cat ~/.ssh/gitlab-id-rsa.pub
$ cat ~/.ssh/github-id-rsa.pub
```


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

---
常见的场景
======
为开源库贡献代码，迟迟无法merge，反复修改，提交历史呈现如下状况

* fork 开源仓库到个人仓库，配置同步fork仓库（配置方法参考上面步骤），完成这些操作后各仓库log如下

```git
remotes/upstream/master  --v1
remotes/origin/your_dev  --v1
your_dev                 --v1
```

* 本地完成了部分修改commit后

```git
remotes/upstream/master  --v1
remotes/origin/your_dev  --v1
your_dev                 --v1--v2
```

* 将本地的commit push到 remotes/origin/your_dev 分支，向forked项目master发起PR

```git
remotes/upstream/master  --v1
remotes/origin/your_dev  --v1--v2
your_dev                 --v1--v2
```

* forked项目master有其他人开发的PR被merge了

```git
remotes/upstream/master  --v1------v3
remotes/origin/your_dev  --v1--v2
your_dev                 --v1--v2
```

* 收到commiter小伙伴的评论，需要修改PR，这里就需要一通骚操作
  
  1. 用git reset --soft v2 将your_dev分支指针指向v1, 这里注意--soft必不可少
  2. 本地做一次新的commit,commit之后效果如下

```git
remotes/upstream/master  --v1------v3
remotes/origin/your_dev  --v1--v2
your_dev                 --v1----------v4
```  

然后利用fetch和merge合并upstream的master分支,此时本地的your_dev分支就更新至upstream的master版本
```bash
git fetch upstream
git merge upstream/master
```

可能会冲突，解决冲突，完成merge，commit后。此时，版本线如下

```git
remotes/upstream/master  --v1------v3
remotes/origin/your_dev  --v1--v2
your_dev                 --v1------v3--v4--v5
```  

对v4,v5作一次rebase操作 ```git rebase -i HEAD~1```

```git
remotes/upstream/master  --v1------v3
remotes/origin/your_dev  --v1--v2
your_dev                 --v1------v3--v6
```

强制push本地your_dev到remotes/origin/your_dev

```git
remotes/upstream/master  --v1------v3
remotes/origin/your_dev  --v1------v3--v6
your_dev                 --v1------v3--v6
```

此时PR，会自动更新成最后一次提交的结果。并且之显示最后一次提交