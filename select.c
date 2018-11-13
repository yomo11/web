#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#define SVR_PORT 8080
#define MAX_SIZE 1024

char web[] =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n\r\n"
    "<!DOCTYPE html>\r\n"
    "<html><head><title>ShellWaveX</title>\r\n"
    "<style>body { background-color: #FF0000 }</style></head>\r\n"
    "<body><center><h1><marquee behavior=alternate scrollamount=10 bgcolor=#00FF00>Welcome!!!</marquee></h1><br>\r\n"
    "<center><h1>test/**/test</h1><br>\r\n"
    "<img src=\"111.jpg\"></center>\r\n"
    "<a href=https://bit.ly/2qMvygt><img src=https://bit.ly/2qMvygt></center></body></html>\r\n"
int main (int argc, char **argv){
	struct sockaddr_in addr;
	int socket_fd,MAX_fd,img_fd,yn=1;
	socklen_t len;
	len=sizeof(struct sockaddr_in);
	fd_set active_fd_set;
	char buf[MAX_SIZE];
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=INADDR_ANY;
	addr.sin_port=htons(SVR_PORT);
	//stop point
	if((socket_fd=socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("socket()");
		return -1;
	}
	else printf("socket_fd=[%d]\n",socket_fd);
	//socket
	if(setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,&yn,sizeof(int))<0){
		perror("setsockopt()");
		return -1;
	}
	//binding
	if(bind(socket_fd,(struct sockaddr *)&addr,sizeof(addr))<0){
		perror("bind()");
		return -1;
	}
	else printf("bind [%s:%u] success\n",inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	//listen
	if(listen(socket_fd,128)==-1) {
		perror("listen()");
		return -1;
	}
	FD_ZERO(&active_fd_set);
	FD_SET(socket_fd,&active_fd_set);
	MAX_fd=socket_fd;
	while(1){
		int ret;
		struct timeval tv;
		fd_set read_fds;
		tv.tv_sec=2;
		tv.tv_usec=0;
		//copy fd set
		read_fds=active_fd_set;
		ret=select(MAX_fd+1,&read_fds,NULL,NULL,&tv);
		if(ret==-1){
            perror("select()");
			return -1;
		}
		else if(ret==0){
			printf("select timeout\n");
			continue;
		}
		else{
			int i;
			//service all sockets
			for(i=0;i<FD_SETSIZE;i++){
				if(FD_ISSET(i,&read_fds)){
					if(i==socket_fd){
						//connection socket
						struct sockaddr_in client_addr;
						int new_fd;
						//accept
						new_fd = accept(socket_fd,(struct sockaddr *)&client_addr,&len);
						if(new_fd==-1){
							perror("accept()");
							return -1;
						}
						else{
							memset(buf,0,1024);
							read(new_fd,buf,1023);
							printf("Accpet client come from [%s:%u] by fd [%d]\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),new_fd);
							if(!strncmp(buf,"GET /111.jpg",19)){
								img_fd=open("111.jpg",O_RDONLY);
								sendfile(new_fd,img_fd,NULL,160000);
								close(img_fd);
							}
							else write(new_fd,web,sizeof(web)-1);
							//fd set(add)
							FD_SET(new_fd,&active_fd_set);
							if(new_fd>MAX_fd) MAX_fd=new_fd;
						}
					}
					else{
						//data arriving on an already-connected socket
						int recv_len;
						//receive
						memset(buf,0,sizeof(buf));
						recv_len=recv(i,buf,sizeof(buf),0);
						if(recv_len==-1){
							perror("recv()");
							return -1;
						}
						else if(recv_len==0) printf("Client disconnect\n");
						else{
							printf("Receive: len=[%d] msg=[%s]\n",recv_len,buf);
							send(i,buf,recv_len,0);
						}
						//clear
						close(i);
						FD_CLR(i,&active_fd_set);
					}
				}
			}
		}
	}
	return 0;
}
