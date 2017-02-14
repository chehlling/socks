/*************************************************************************
	> File Name: comm.cpp
	> Author: chehlling
	> Mail: chehlling@126.com 
	> Created Time: Sun 13 Nov 2016 11:16:12 PM PST
 ************************************************************************/

#include "comm.h"
using namespace std;                                                                                                                  
  
int recv_data(int sock,void* buf,size_t len)
{
	char* data_buf=(char*)buf;
	int ret=0;
	int size=len;
	int left=size;
	while(left){
		ret=recv(sock,data_buf+size-left,left,0);
		if(ret<=0){
			perror("recv");
			return -1; 
		}
		left-=ret;								   
	}   
	return 0;
}
  
int send_data(int sock,void* buf,size_t len)
{
	char* data_buf=(char*)buf;
	int ret=0;
	int size=len;
	int left=size;
	while(left){
		int ret=0;
		ret=send(sock,(char*)buf+size-left,left,0);
		if(ret<=0){
			perror("send");
			return -1;													
		}
		left-=ret;
	}
	data_buf[size-left]='\0';
	return 0;
}
