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



char web[] =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n\r\n"
    "<!DOCTYPE html>\r\n"
    "<html><head><title>ShellWaveX</title>\r\n"
    "<style>body { background-color: #FF0000 }</style></head>\r\n"
    "<body><center><h1><marquee behavior=alternate scrollamount=10 bgcolor=#00FF00>Welcome!!!</marquee></h1><br>\r\n"
    "<center><h1>test/**/test</h1><br>\r\n"
    "<img src=\"111\"></center>\r\n"
    "<a href=https://bit.ly/2qMvygt><img src=https://bit.ly/2qMvygt></center></body></html>\r\n"
int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr,client_addr;
    socklen_t sin_len=sizeof(client_addr);
    sever_fd=socket(AF_INET,SOCK_STREAM,0);
    int sever_fd,client_fd,img_fd,tp,on=1;
    char buf[2048];
    if(sever_fd<0){
        perror("socket");
        exit(1);
    }
    setsockopt(sever_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(int));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=INADDR_ANY;
    server_addr.sin_port=htons(8080);
    if(bind(sever_fd,(struct sockaddr *)&server_addr,sizeof(server_addr))==-1){
        perror("bind");
        close(sever_fd);
        exit(1);
    }
    if(listen(sever_fd,20)==-1){
        perror("listen");
        close(sever_fd);
        exit(1);
    }
    while(1){
        client_fd=accept(sever_fd,(struct sockaddr *)&client_addr,&sin_len);
        if(client_fd==-1){
            perror("Connection failed...\n");
            continue;
        }
        printf("Got client connection...\n");
        tp=fork();
        if(!tp){
            //child process
            close(sever_fd);
            memset(buf,0,2048);
            read(client_fd,buf,2047);
            printf("%s\n",buf);
            if(!strncmp(buf,"GET /111",16)){
                img_fd=open("111",O_RDONLY);
                sendfile(client_fd,img_fd,NULL,160000);
                close(img_fd);
            }
            else write(client_fd,web,sizeof(web)-1);
            close(client_fd);
            printf("closing...\n");
            exit(0);
        }
        //parent process
        if(tp){
            wait(NULL);
            close(client_fd);
        }
    }
    return 0;
}
