#include <stdlib.h>  
#include <stdio.h>  
#include <stddef.h>  
#include <sys/socket.h>  
#include <netinet/in.h>
#include <sys/un.h>  
#include <errno.h>  
#include <string.h>  
#include <unistd.h> 
#include <fcntl.h>

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
			printf("connecting error=%d \n", EINPROGRESS);

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
					   printf("getsockopt ret == 0 , error:%d\n", error);
					   if( error == 0){
				          retval = 0;
						  printf("connect to %s:%d \n", szip, usport);
					   }
				    }   
				}
            }
		}
	}
	close(sock);
	return retval;
}


int l0006_netcom_iscanconnet2(const char *szip, unsigned short usport)
{
	int sockfd,recvbytes,res,flags,error,n;
	socklen_t len;
	fd_set rset,wset;
	struct timeval tval;
	tval.tv_sec=0;
	tval.tv_usec=300000;
	struct sockaddr_in serv_addr;
	char*sendData="1234567890";//发送字符串
	char buf[1024]="/0"; //接收buffer
	//创建socket描述符
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket create failed");
		return 1;
	}

	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(usport);
	serv_addr.sin_addr.s_addr=inet_addr(szip);
	bzero(&(serv_addr.sin_zero),8);
	flags=fcntl(sockfd,F_GETFL,0);
	fcntl(sockfd,F_SETFL,flags|O_NONBLOCK);//设置为非阻塞
	if( (res = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) )< 0)
	{
		if(errno != EINPROGRESS){
			return	1;
		}
	}

   //如果server与client在同一主机上，有些环境socket设为非阻塞会返回 0
	if(0 == res) 
		goto done;
	FD_ZERO(&rset);
	FD_SET(sockfd,&rset);
	wset=rset;
	if( ( res = select(sockfd+1, NULL, &wset, NULL,&tval) ) <= 0)
	{
		perror("connect time out/n");
		close(sockfd);
		return	1;
	}else{
		len=sizeof(error);
		getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
		if(error){
			fprintf(stderr, "Error in connection() %d - %s/n", error, strerror(error));
			return 1;
		}
	}
	
done:
	if( (n = send(sockfd, sendData, strlen(sendData),0) ) ==-1 )
	{
		perror("send error!");
		close(sockfd);
		return 1;
	}
	if( ( n = select(sockfd+1,&rset,NULL, NULL,&tval)) <= 0 )//rset没有使用过，不用重新置为sockfd
	{
		perror("receive time out or connect error");
		close(sockfd);
		return -1;
	}
	if((recvbytes = recv(sockfd, buf, 1024, 0)) ==-1)
	{
		perror("recv error!");
		close(sockfd);
		return	1;
	}
	printf("receive num %d/n",recvbytes);
	printf("%s/n",buf);

	return 0;
}


int main(int argc, char *argv[])
{
	if( argc != 3){
		printf("please input ip and port!!! \n");
		return -1;
	}

	int retval = l0006_netcom_iscanconnet(argv[1], atoi(argv[2]));
	printf("\n l0006_netcom_iscanconnet  retval:%d \n", retval);

	retval = l0006_netcom_iscanconnet2(argv[1], atoi(argv[2]));
	printf("\n l0006_netcom_iscanconnet2  retval:%d \n", retval);
	return 0;
}
