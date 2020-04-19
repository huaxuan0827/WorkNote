git开发环境搭建

# 1、window10 git安装
https://gitforwindows.org/ 下载的，但是没有.ssh目录，一顿操作之后又有了，奇怪哟西。

# 2、linux-ubuntu64 git安装

# 3、window10 配置 gitee和github

Github、Gitee同时配置SSH
## 1. 进入ssh目录
cd ~/.ssh  
## 2. 使用命令分别创建两个平台的公钥
ssh-keyen -t rsa -C "xxxxxx@xxx.com" -f "id_rsa_gitee"  
ssh-keyen -t rsa -C "xxxxxx@xxx.com" -f "id_rsa_github"  
完成后，目录内容如下：  
-rw------- 1 lambda lambda 1679 Jun 14 10:51 id_rsa_gitee  
-rw-r--r-- 1 lambda lambda  400 Jun 14 10:51 id_rsa_gitee.pub  
-rw------- 1 lambda lambda 1679 Apr  1 13:44 id_rsa_github  
-rw-r--r-- 1 lambda lambda  399 Apr  1 13:44 id_rsa_github.pub  
-rw-r--r-- 1 lambda lambda 2434 Jun 14 11:05 known_hosts  
## 3. 将产生的公钥xxx.pub内容分别配置到Github和Gitee。
## 4. 创建config文件，解决SSH冲突
在.ssh目录下创建config文件，添加如下内容：  
#github  
Host github.com  
HostName github.com  
PreferredAuthentications publickey  
IdentityFile ~/.ssh/id_rsa_github  
#gitee  
Host gitee.com  
HostName gitee.com  
PreferredAuthentications publickey  
IdentityFile ~/.ssh/id_rsa_gitee  


## 5. 测试
命令：  
ssh -T git@gitee.com  
#或  
ssh -T git@github.com  
成功：  
Hi xxxx! You've successfully authenticated, but GITEE.COM does not provide shell access.  

## 6、 遇到问题
在测试环节，有时会出现如下错误：  
#Linux  
Bad owner or permissions on /home/lambda/.ssh/config  
#Windows  
Bad owner or permissions on C:\\Users\\Ran\\.ssh\\config  

解决办法  
Linux  
sudo chmod 600 ~/.ssh/config  
Windows  
1. 查看PATH环境中是否存在C:\Windows\System32\OpenSSH\ssh.exe   
2. 将C:\Windows\System32\OpenSSH\ssh.exe改成C:\Program Files\Git\usr\bin\ssh.exe  

# 4、linux-ubuntu64 配置github gitee


# 5、winodow相关工具

# 6、git error
## 1，不能连接22端口 换一个端口
ssh: connect to host github.com port 22: Connection refused  
fatal: Could not read from remote repository.  
  
Please make sure you have the correct access rights  
and the repository exists.  
  
vim ~/.ssh/config  

#github  
#Host github.com  
#HostName github.com  
#PreferredAuthentications publickey  
#IdentityFile ~/.ssh/id_rsa_github  
  
#github-new  
Host github.com  
Hostname ssh.github.com  
PreferredAuthentications publickey  
IdentityFile ~/.ssh/id_rsa_github  
Port 443  

#gitee  
Host gitee.com  
HostName gitee.com  
PreferredAuthentications publickey  
IdentityFile ~/.ssh/id_rsa_gitee  
  
ssh -T git@github.com and ok  
  
## 2, git clone时出现Permission denied (public key)问题解决
ssh -T -v git@github.com 可以调试连接过程；   
debug1: Authentications that can continue: publickey  
debug1: Next authentication method: publickey  
debug1: Trying private key: /c/Users/86187/.ssh/id_rsa  
debug1: Trying private key: /c/Users/86187/.ssh/id_dsa  
debug1: Trying private key: /c/Users/86187/.ssh/id_ecdsa  
debug1: Trying private key: /c/Users/86187/.ssh/id_ecdsa_sk  
debug1: Trying private key: /c/Users/86187/.ssh/id_ed25519  
debug1: Trying private key: /c/Users/86187/.ssh/id_ed25519_sk  
debug1: Trying private key: /c/Users/86187/.ssh/id_xmss  
debug1: No more authentication methods to try.  
git@github.com: Permission denied (publickey).   
试了一通之后还是不行，最后发现config配置文件 搞成 config.txt了  
ssh-add -l #命令查看密钥列表，好像没有用耶，好像是没有启用代理，有时间再研究吧！  
  
## 3，为什么浏览页面这么丑，换行符没有弄好，有时间研究下。
