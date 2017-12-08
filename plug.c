#include <sys/types.h>    
#include <sys/socket.h>    
#include <asm/types.h>    
#include <linux/netlink.h>    
#include <linux/rtnetlink.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <string.h>
#include <signal.h>
#include <sys/epoll.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/prctl.h>

#define _DEBUG
#ifdef _DEBUG
#define debug(format, ...) printf(format, ##__VA_ARGS__)
#else
#define debug(format...)
#endif

//#define EXIT(n) _exit(n)
#define BUFLEN 20480
#define MAX_EVENTS 2

#define UP_PATH "/home/bcc/Scripts/set_net_dhcp.sh"
#define DOWN_PATH "/home/bcc/Scripts/set_net_down.sh"

volatile bool exit_thread = false;
volatile int socketFd = -1;
pthread_t mythread;

int process_data(char *buff,int num);
void thread_set_name(char *name);

void modify_event(int epollfd,int fd,int state)
{
     struct epoll_event ev;
     ev.events = state;
     ev.data.fd = fd;
     epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);
}

void add_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
}

void delete_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev);
}

static void myExitHandler (int sig)
{
	void *retval = NULL;
	
	debug("App termiated!\r\n");
	/*if(socketFd > 0) {
		delete_event(epfd,tmpSockFd,EPOLLIN|EPOLLET);
	}*/
	exit_thread = true;
	int ret = pthread_join(mythread, &retval);
	if (ret < 0) {
		perror("cannot join with thread1\n");
	}
	debug("Done with app termiated!\r\n");
	//_exit(0);
}

//static int epoll_data() 
void checking_plug_thread(void *ptr)
{
	int nfds = -1,tmpSockFd=-1;
	int len = -1;
	struct epoll_event ev,events[MAX_EVENTS];
	char buf[BUFLEN];
	
	thread_set_name("plug_thread");

	int epfd = epoll_create(5);
	
	add_event(epfd,socketFd,EPOLLIN|EPOLLET);
	
	 while (!exit_thread) {
		 nfds=epoll_wait(epfd,events,MAX_EVENTS,500);
		 for(int i=0;i<nfds;++i) {
			 /*if(events[i].data.fd == socketFd) {
				printf("[epoll]:Ipc server begin accept connections!\r\n");
				connFd = pIpc->Accept();
				printf("[Ipc epoll]:connfd:%d\r\n",connFd);
				if(connFd < 0) {
					perror("[Ipc epoll]:connfd <0");
					_exit(1);
				}
				add_event(epfd,connFd,EPOLLIN|EPOLLET);
			 } else */
			if(events[i].events&EPOLLIN) { //read
				tmpSockFd = events[i].data.fd;
				if(tmpSockFd == socketFd) {
					debug("[epoll]:pollin,fd:%d\r\n",tmpSockFd);
					len = read(tmpSockFd, buf, BUFLEN);
					if(len > 0) {
						debug("received str len:%d\r\n",len);
						process_data(buf,len);
						//debug("str is [%s]\r\n",buf);
						memset(buf,0,len);	
					} else if(len == 0) {
						debug("[Ipc epoll] close fd\r\n");
						delete_event(epfd,tmpSockFd,EPOLLIN|EPOLLET);
						shutdown(tmpSockFd,SHUT_RDWR);
						close(tmpSockFd);
						tmpSockFd = -1;
						socketFd = -1;
					}
				}
			} else if(events[i].events&EPOLLOUT) { //write
				tmpSockFd = events[i].data.fd;
				debug("[Ipc epoll]:pollout:%d\r\n",tmpSockFd);
			}
		 }
	 }
	 
	delete_event(epfd,socketFd,EPOLLIN|EPOLLET);
	//shutdown(socketFd,SHUT_RDWR);
	close(socketFd);
	socketFd = -1;
	
	debug("end of plug_thread\r\n");
}

int process_data(char *buff,int num)
{
	struct nlmsghdr *nh;
	struct ifinfomsg *ifinfo;
	struct rtattr *attr;
	int len = BUFLEN;
	char tempbuf[100]={0};
	
	//debug("process_data\r\n");
	if(NULL == buff)
		return -1;

	for (nh = (struct nlmsghdr *)buff; NLMSG_OK(nh, num); nh = NLMSG_NEXT(nh, num)) {
		if (nh->nlmsg_type == NLMSG_DONE)
			break;
		else if (nh->nlmsg_type == NLMSG_ERROR) 
			return -1;
		else if (nh->nlmsg_type != RTM_NEWLINK)  
			continue;
		ifinfo = NLMSG_DATA(nh);
		if(ifinfo->ifi_flags & IFF_LOWER_UP) {
			debug("%u: up\r\n", ifinfo->ifi_index);
			sprintf(tempbuf,"/bin/sh %s",UP_PATH);
			//debug("%s\r\n",tempbuf);
			system(tempbuf);
			memset(tempbuf,0,100);
		} else {
			debug("%u: down\r\n", ifinfo->ifi_index);
			memset(tempbuf,0,100);
		}
		
		/*printf("%u: %s", ifinfo->ifi_index,
				(ifinfo->ifi_flags & IFF_LOWER_UP) ? "up" : "down" );*/
				
		/*attr = (struct rtattr*)(((char*)nh) + NLMSG_SPACE(sizeof(*ifinfo)));
		len = nh->nlmsg_len - NLMSG_SPACE(sizeof(*ifinfo));
		
		for (; RTA_OK(attr, len); attr = RTA_NEXT(attr, len))
		{
			if (attr->rta_type == IFLA_IFNAME)
			{
				printf(" %s", (char*)RTA_DATA(attr));
				break;
			}
		}
		printf("\n");*/
	}
}

int main(int argc, char *argv[])
{    
    //int fd = -1;
    //char buf[BUFLEN] = {0};
    int len = -1;
    struct sockaddr_nl addr;
	int ret;
	void *retval = NULL;
    //struct nlmsghdr *nh;
    //struct ifinfomsg *ifinfo;
    //struct rtattr *attr;    

	signal (SIGINT, myExitHandler);
	signal (SIGQUIT, myExitHandler);
 	//signal (SIGILL, myExitHandler);
  	signal (SIGABRT, myExitHandler);
  	//signal (SIGFPE, myExitHandler);
  	signal (SIGKILL, myExitHandler);
	//signal (SIGPIPE, myExitHandler);
	signal (SIGTERM, myExitHandler);

    socketFd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    setsockopt(socketFd, SOL_SOCKET, SO_RCVBUF, &len, sizeof(len));
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTNLGRP_LINK;
    bind(socketFd, (struct sockaddr*)&addr, sizeof(addr));
	
	ret = pthread_create(&mythread, NULL,(void *)&checking_plug_thread,NULL);
	if(ret < 0) {
		perror("creat thread failed!\r\n");
		_exit(-1);
	}
	
	ret = pthread_join(mythread, &retval);
	//printf("thread1 return value(retval) is %d\n", (int)retval);
	//printf("thread1 return value(tmp) is %d\n", tmp1);
	if (ret < 0) {
		perror("cannot join with thread1\n");
	}
	debug("main app end\n");
   /* while ((retval = read(fd, buf, BUFLEN)) > 0)
    {
        for (nh = (struct nlmsghdr *)buf; NLMSG_OK(nh, retval); nh = NLMSG_NEXT(nh, retval))
        {
            if (nh->nlmsg_type == NLMSG_DONE)
                break;
            else if (nh->nlmsg_type == NLMSG_ERROR)    
                return -1;    
            else if (nh->nlmsg_type != RTM_NEWLINK)    
                continue;    
            ifinfo = NLMSG_DATA(nh);    
            printf("%u: %s", ifinfo->ifi_index,    
                    (ifinfo->ifi_flags & IFF_LOWER_UP) ? "up" : "down" );    
            attr = (struct rtattr*)(((char*)nh) + NLMSG_SPACE(sizeof(*ifinfo)));    
            len = nh->nlmsg_len - NLMSG_SPACE(sizeof(*ifinfo));    
            for (; RTA_OK(attr, len); attr = RTA_NEXT(attr, len))    
            {    
                if (attr->rta_type == IFLA_IFNAME)    
                {    
                    printf(" %s", (char*)RTA_DATA(attr));    
                    break;    
                }    
            }    
            printf("\n");
        }
    }*/
    return 0;
}

void thread_set_name(char *name)
{
	char tname[100] = {0};
	
	if(name == NULL)
		return;
	//memset(tname, 0, 100);
	sprintf(tname,"%s",name);
	prctl(PR_SET_NAME, tname);
}