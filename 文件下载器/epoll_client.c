#include<my_socket.h>
#include<sys/epoll.h>
#define IP "10.211.55.6"
#define PORT 1030
void request_data(int sockfd,char* path);
int main(int argc,char* argv[])
{
	int fd_server=socket(AF_INET,SOCK_STREAM,0);
	MY_ASSERT(fd_server!=-1,"socket_error\n");
	socket_connect(fd_server,IP,atoi(argv[1]));
	printf("connect successful!\n");
	request_data(fd_server,argv[2]);
}
void request_data(int sockfd,char* path)
{
	int len=0;
	char buf[1024];
	char buf2[1024];
	memset(buf2,0,1024);
	memset(buf,0,1024);
	socket_recv(sockfd,(char*)&len,4);
	socket_recv(sockfd,buf,len);
	if(strcmp(buf,"over")==0)
	{
		printf("download task is over!\n");
		close(sockfd);
		exit(1);
	}
	char* str=strrchr(buf,'/');
	sprintf(buf2,"%s%s",path,str);
	int fd=open(buf2,O_WRONLY|O_CREAT,0666);
	int count=0;
	while(memset(buf,0,1024))
	{
		socket_recv(sockfd,(char*)&len,4);
		if(len==0)
		{
			printf("recv file :%s size: %d\n",buf2,count);
			request_data(sockfd,path);
		}
		socket_recv(sockfd,buf,len);
		count+=len;
		write(fd,buf,len);
	}
}
