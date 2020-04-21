参考网址：
 https://blog.csdn.net/jctian000/article/details/80306267

 https://blog.csdn.net/qq_40194498/article/details/80245607

 https://blog.csdn.net/gogor/article/details/5897117?depth_1-utm_source=distribute.pc_relevant.none-task-blog-BlogCommendFromBaidu-3&utm_source=distribute.pc_relevant.none-task-blog-BlogCommendFromBaidu-3

# linux 客户端 Socket 非阻塞connect编程

## 最终提交的代码
int l0006_netcom_iscanconnet(const char *szip, unsigned short usport)
{
	int sock;
	struct sockaddr_in serv_addr;
	int retval = -1;

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1){
		return -1;
	}
	int options = fcntl(sock, F_GETFL, 0); 
        fcntl(sock, F_SETFL, options | O_NONBLOCK); 
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(szip);
	serv_addr.sin_port = htons(usport);

	retval = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	printf("try connect to %s:%d retval:%d, errno:%d\n",szip, usport, retval, errno);
	if( retval < 0){
		if( errno == EINPROGRESS){
			fd_set writefds;
            FD_ZERO(&writefds);
            FD_SET(sock, &writefds);                  

            struct timeval timeout;         
            timeout.tv_sec = 3; 
            timeout.tv_usec = 0;     
            int ret = select(sock + 1, NULL, &writefds, NULL, &timeout);
			printf("select ret = %d \n", ret);
            if( ret > 0){
            	if(FD_ISSET(sock, &writefds)){
                   int error = 0;
    			   socklen_t length = sizeof(error);
				    if(getsockopt(sock,SOL_SOCKET,SO_ERROR,&error,&length) == 0){
					   printf("getsockopt ret == 0 \n");
					   if( error == 0)
				               retval = 0;;
				    }   
				}
            }
		}
	}
	close(sock);
	return retval;
}
# connect 流程
我们知道，tcp客户端要与服务端通信，必须先建立连接，即调用connect函数完成三次握手，而默认情况下connect是阻塞方式的，也就是说调用connect函数会发生阻塞，超时时间可能在75s至几分钟之间。当然同一主机除外，同一主机上调用connect通常会立即成功。

为避免长时间的connect阻塞，可以使用如下非阻塞connect方式来处理：
一、 创建socket,返回套接口描述符 
二、 调用fcntl把套接口描述符设置成非阻塞 
三、 调用connect开始建立连接   
四、 判断连接是否成功建立                  
        A: 如果connect返回0,表示连接成功(服务器和客户端在同一台机器上时就有可能发生这种情况)                
        B: 调用select来等待连接建立成功完成                                
	    (Berkeley的实现(和Posix.1g)有两条与select和非阻塞IO相关的规则: 
	     1.当连接建立成功时,套接口描述符变成可写;
	     2.当连接出错时,套接口描述符变成既可读又可写;
	     注意:当一个套接口出错时,它会被select调用标记为既可读又可写;)
五、 继续判断select返回值
         如果select返回0,则表示建立连接超时; 我们返回超时错误给用户，同时关闭连接，以防止三路握手操作继续进行下去
         如果select返回大于0的值,则需要检查套接口描述符是否可读或可写;如果套接口描述符可读或可写,则我们可以通过调用getsockopt来得到套接口上待处理的错误(SO_ERROR),如果连接建立成功,这个错误值将是0,如果建立连接时遇到错误,则这个值是连接错误所对应的errno值(比如:ECONNREFUSED,ETIMEDOUT等).

# 问题
按照unix网络编程的描述，当网络发生错误的时候，getsockopt返回-1，return -1，程序结束。网络正常时候返回0，程序继续执行。
       可是我在linux下，无论网络是否发生错误，getsockopt始终返回0，不返回-1，说明linux与unix网络编程还是有些细微的差别。就是说当socket描述符可读可写的时候，这段代码不起作用。不能检测出网络是否出现故障。
      我测试的方法是，当调用connect后，sleep（2）休眠2秒，借助这两秒时间将网络助手断开连接，这时候select返回2，说明套接口可读又可写，应该是网络连接的出错情况。
      此时，getsockopt返回0，不起作用。获取errno的值，指示为EINPROGRESS，没有返回unix网络编程中说的ENOTCONN，EINPROGRESS表示正在试图连接，不能表示网络已经连接失败。
      针对这种情况，unix网络编程中提出了另外3种方法，这3种方法，也是网络上给出的常用的非阻塞connect示例：
    a.再调用connect一次。失败返回errno是EISCONN说明连接成功，表示刚才的connect成功，否则返回失败。 代码如下：*/
intconnect_ok;
connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr) );
switch(errno)
{
caseEISCONN:   //connect ok
printf("connect OK/n");
connect_ok=1;
break;
caseEALREADY:
connect_0k=-1
break;
caseEINPROGRESS:// is connecting, need to check again
connect_ok=-1
break;
default:
printf("connect fail err=%d/n",errno);
connect_ok=-1;
break;
}
/*如程序所示，根据再次调用的errno返回值将connect_ok的值，来进行下面的处理，connect_ok为1继续执行其他操作，否则程序结束。

    但这种方法我在linux下测试了，当发生错误的时候,socket描述符（我的程序里是sockfd）变成可读且可写，但第二次调用connect 后，errno并没有返回EISCONN，,也没有返回连接失败的错误，仍旧是EINPROGRESS，而当网络不发生故障的时候，第二次使用 connect连接也返回EINPROGRESS，因此也无法通过再次connect来判断连接是否成功。
     b.unix网络编程中说使用read函数，如果失败，表示connect失败，返回的errno指明了失败原因，但这种方法在linux上行不通，linux在socket描述符为可读可写的时候，read返回0，并不会置errno为错误。
       c.unix网络编程中说使用getpeername函数，如果连接失败，调用该函数后，通过errno来判断第一次连接是否成功，但我试过了，无论网络连接是否成功，errno都没变化，都为EINPROGRESS，无法判断。
       悲哀啊，即使调用getpeername函数，getsockopt函数仍旧不行。
     综上方法，既然都不能确切知道非阻塞connect是否成功，所以我直接当描述符可读可写的情况下进行发送，通过能否获取服务器的返回值来判断是否成功。(如果服务器端的设计不发送数据，那就悲哀了。)
    程序的书写形式出于可移植性考虑，按照unix网络编程推荐写法，使用getsocketopt进行判断，但不通过返回值来判断，而通过函数的返回参数来判断。
