https://blog.csdn.net/jctian000/article/details/80306267
https://blog.csdn.net/qq_40194498/article/details/80245607

int l0006_netcom_iscanconnet(const char *szip, unsigned short usport)
{
	int sock;
	struct sockaddr_in serv_addr;
	int retval = -1;

	return 0;

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
				       retval = 0;;
				    }   
				}
            }
		}
	}
	close(sock);
	return retval;
}
