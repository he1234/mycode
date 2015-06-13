#include<my_socket.h>
#include<sys/epoll.h>
#include<signal.h>
#define IP "10.211.55.6"
#define PORT 1030
void add_epoll(int epollfd,int sockfd,int state);
void handle_request(int sockfd,char* path);
void handle_dir(int fd_client,char* dir_path);
void handle(int num)
{
    perror("pipe\n");
}
int  set_nonblock(int sockfd)
{
    int old=fcntl(sockfd,F_GETFL);
    old|=O_NONBLOCK;
    fcntl(sockfd,F_SETFL,old);
    return 0;
}
int main(int argc,char* argv[])
{
    signal(SIGPIPE,handle);
    int fd_listen=socket_tcp(IP,atoi(argv[1]));
    socket_listen(fd_listen,128);
    struct epoll_event events[1024];
    memset(events,0,1024);
    int epollfd;
    MY_ASSERT((epollfd=epoll_create(1024))!=-1,"epoll_create");
    set_nonblock(fd_listen);
    add_epoll(epollfd,fd_listen,EPOLLIN|EPOLLET);
    printf("listening.....\n");
    while(1)
    {
        int index;
        int ret=epoll_wait(epollfd,events,1024,-1);
        for(index=0;index<ret;index++)
        {
            if(events[index].data.fd==fd_listen)
            {
                char client_ip[1024];
                int client_port;
                int fd_client=socket_accept(fd_listen,client_ip,&client_port);
                printf("client(ip:%s:%d) is connect!\n",client_ip,client_port);
                set_nonblock(fd_client);
                add_epoll(epollfd,fd_client,EPOLLOUT|EPOLLET);  
            }
            else
            {
                handle_dir(events[index].data.fd,argv[2]);                
                char* buf="over";
                int len=strlen(buf);
                socket_send(events[index].data.fd,(char*)&len,4);
                socket_send(events[index].data.fd,buf,len);
                printf("transport task is over!\n");
                MY_ASSERT((epoll_ctl(epollfd,EPOLL_CTL_DEL,events[index].data.fd,NULL))==0,"epoll_delete");
                close(events[index].data.fd);
            }
        }
    }
}
void add_epoll(int epollfd,int sockfd,int state)
{
    struct epoll_event my_event;
    my_event.events=state;
    my_event.data.fd=sockfd;
    MY_ASSERT((epoll_ctl(epollfd,EPOLL_CTL_ADD,sockfd,&my_event))==0,"epoll_add");
}
void handle_request(int sockfd,char* path)
{
    int len=strlen(path);
    socket_send(sockfd,(char*)&len,4);
    socket_send(sockfd,path,len);
    char buf[1024];
    int fd=open(path,O_RDONLY);
    int count=0;
    while(memset(buf,0,1024),(len=read(fd,buf,1024))>0)
    {
        socket_send(sockfd,(char*)&len,4);
        socket_send(sockfd,buf,len);
        count+=len;
    }
    printf("send file:%s  size:%d\n",path,count);
    socket_send(sockfd,(char*)&len,4);
}
void handle_dir(int fd_client,char* dir_path)
{ 
    struct dirent* p_entry;
    struct stat my_stat;
    DIR* dir;
    memset(&my_stat,0,sizeof(my_stat));//分析该目录状态
    if(-1==lstat(dir_path,&my_stat))
    {
        perror("lstat\n");
    }
    if(S_ISREG(my_stat.st_mode))//如果是普通文件，直接解析
    {
        handle_request(fd_client,dir_path);
    }
    else if(S_ISDIR(my_stat.st_mode))//如果是目录打开
    {
        dir=opendir(dir_path);
        if(dir==NULL)
        {
            perror("dir open failed\n");
        }
        while(memset(&my_stat,0,sizeof(my_stat)),(p_entry=readdir(dir))!=NULL)
        {
            if(strcmp(".",p_entry->d_name)==0||strcmp("..",p_entry->d_name)==0)
                continue;
            char buf[1024];
            sprintf(buf,"%s/%s",dir_path,p_entry->d_name);//将目录绝对路径赋值buf
            handle_dir(fd_client,buf);
        }
    }
}


