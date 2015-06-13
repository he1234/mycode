//epoll_chat.hpp
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "socket.hpp"
#include "thread_chat.hpp"
#include <vector>
#include <json/json.h>
#include <sstream>
#define MAXSIZE 1024
namespace EPOLL
{
	class Epoll
	{
		public:
			Epoll(const std::string port):socket(new SOCKET::Socket(port))
			{
				chat=new FACTORY::Factory(socket);   
			}
			void epoll_init()
			{
				epoll_fd=epoll_create(MAXSIZE);
			}
			void event_add(const int& sockfd,const int& state)
			{
				struct epoll_event ev;
				ev.events=state;
				ev.data.fd=sockfd;
				epoll_ctl(epoll_fd,EPOLL_CTL_ADD,sockfd,&ev);
			}
			void event_delete(const int& sockfd)
			{
				epoll_ctl(epoll_fd,EPOLL_CTL_DEL,sockfd,NULL);
			}
			void event_mod(const int& sockfd,const int& state)
			{
				struct epoll_event ev;
				ev.events=state;
				ev.data.fd=sockfd;
				epoll_ctl(epoll_fd,EPOLL_CTL_MOD,sockfd,&ev);
			}

			//处理epoll_wait()返回的就绪链表中的epoll_event数据数组
			void handle_request(const int& active)
			{
				if(active==0)
				{
					std::cout<<"epoll listen system timeout !"<<std::endl;
				}
				for(int index=0;index<active;index++)
				{
					if(socket->server_sock==events[index].data.fd)
					{
						int fd_client=socket->socket_accept();
						chat->info->online_client.push_back(fd_client);//把fd_client放入上线客户端
						chat->handle->put_sockfd(fd_client);//把fd_client放入工作队列
					}
				}
			}
			void server_init()//服务器初始化逻辑
			{
				socket->socket_init();
				socket->set_nonblock(socket->server_sock);//设置非阻塞套接口
				event_add(socket->server_sock,EPOLLIN|EPOLLET);//epoll_event中events设置为EPOLLET边缘触发模式，所以必须使用非阻塞套接口
				socket->socket_bind();
				socket->socket_listen();
			}
			void epoll_listen()//使用epoll模型的服务器主逻辑
			{
				epoll_init();	//epoll初始化
				server_init();	//服务器初始化
				int active=0;
				chat->on();		//启动服务器聊天线程
				std::cout<<"waiting for connecting......."<<std::endl;
				while(1)
				{
					active=epoll_wait(epoll_fd,events,MAXSIZE,50000);
					handle_request(active);
				}
			}
		private:
			SOCKET::Socket* socket;
			int epoll_fd;
			struct epoll_event events[MAXSIZE];
			FACTORY::Factory* chat;
	};
}

