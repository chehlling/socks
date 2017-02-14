/*************************************************************************
	> File Name: server.cpp
	> Author: chehlling
	> Mail: chehlling@126.com 
	> Created Time: Sun 13 Nov 2016 11:20:28 PM PST
 ************************************************************************/

#include <iostream>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>
#include <netdb.h>
#include <stdlib.h>
#include "protocol.h"                                                                                                                 
#include "comm.h"
#include <assert.h>
  
extern int h_errno;
  
using namespace std;
  
struct ThreadInfo
{
	    int in; 
		    int out;
			    pthread_t id; 
};
void proc_handler(int sock);
void* thre_handler(void* argc);
     
int main(int argc,char* argv[])
{   
	if(argc!=2){
		cout<<"Usage:[proc] [port] "<<endl;
		return -1;
	}
	
	int listen_sock=socket(AF_INET,SOCK_STREAM,0);
	if(listen_sock<0){
		perror("socket");
	}
	sockaddr_in local;                                                                                                                
	local.sin_family=AF_INET;
	local.sin_addr.s_addr=INADDR_ANY;
	//local.sin_addr.s_addr=inet_addr(argv[1]);
	local.sin_port=ntohs(atoi(argv[1]));
	socklen_t len=sizeof(local);
	if(0!=bind(listen_sock,(sockaddr*)&local,len)){
		perror("bind");
		return -1;
	}
	if(0!=listen(listen_sock,5)){
		perror("listen");
		return -1;
	}
	int done=0;
	while(!done){
		sockaddr_in cli_addr;
		socklen_t len;
		int sock=accept(listen_sock,(sockaddr*)&cli_addr,&len);                                                             
		cout<<"connection new sock:"<<inet_ntoa(cli_addr.sin_addr)<<endl;                                                        
		pid_t pid1=fork();
		if(pid1<0){
			perror("fork");
			return -1;
		}else if(pid1>0){
			waitpid(pid1,NULL,0);
		}else{
			pid_t pid2=fork();
			if(pid2<0){
				perror("fork");
				return -1;
			}else if(pid2>0){
				return 0;
			}else{
				proc_handler(sock);                                                                                         
				close(sock);
				return 0;
			}
		}
	}
	return 0;
}

void proc_handler(int in)//in是和客户端通信的socket
{   
	cout<<"proc_handler start..."<<endl;
	//VER     NMETHODS     METHODS 
	// 1          1       1 to 255 
	startReq req;
	if(0!=recv_data(in,&req,sizeof(req))){
		cout<<"recv error in protocol!"<<endl;
		return;
	}   
	if(req.ver!=5 || req.n_methods<1){
		cout<<"recv error in protocol!"<<endl;
		return;
	}
	                                                 
	//选择methods,八位组0-255                                                                                                         
	startRep rep;
	// VER    METHOD 
	//  1       1 
	char methods[256];
	if(req.n_methods==1){
		if(0!=recv_data(in,methods,1)){
			cout<<"recv error in protocol!"<<endl;
			return;
		}
		if(methods[0]==0xFF){//FF没有可接受的方法
			return;
		}
		else if(methods[0]==0x00){//不需要认证
			rep.ver=5;
			rep.method=0;
			if(0!=send_data(in,&rep,sizeof(rep))){
				cout<<"send error in protocol!"<<endl;
				return;
			}                                                                                                                   
		}else{
		}
	}else{//选择methods
	}
	//接受目标server addr
	// VER    CMD    RSV    ATYP    DST.ADDR    DST.PROT 
	//  1      1     X’00’   1      Variable      2
	addrReq reqaddr;
	if(0!=recv_data(in,&reqaddr,sizeof(reqaddr))){
		cout<<"recv error in dstaddr!"<<endl;
		return;
	}
	if(reqaddr.ver!=5 ||reqaddr.rsv!=0){
		cout<<"recv error in dstaddr!"<<endl;
		return;
	}

	sockaddr_in remote;
	remote.sin_family=AF_INET;                                                                                                        

	if(reqaddr.atyp==1){//ipv4
		cout<<"accept dst_addr ipv4"<<endl;
		in_addr_t dstip;
		if(0!=recv_data(in,&remote.sin_addr.s_addr,4)){
			cout<<"recv error in dstaddr!"<<endl;
			return;
		}
	}else if(reqaddr.atyp==3){//域名
		cout<<"accept dst_addr domain"<<endl;
		char domain[SIZE];
		if(0!=recv_data(in,&domain,sizeof(domain))){
			cout<<"recv error in dstaddr!"<<endl;
			return;
		}
		cout<<"connect to domain: "<<domain<<endl;
		struct hostent* ht;
		if((ht=gethostbyname(domain))==NULL){
			herror("gethostbyname");
			return;
		}                                                                                                                             
		memcpy(&remote.sin_addr.s_addr,ht->h_addr_list[0],sizeof(remote.sin_addr.s_addr));//web服务端可能有多个ip，选择一个
	}else{//ipv6 or error
		return;
	}                                                                                                       
	//获取目的port
	if(0!=recv_data(in,&remote.sin_port,2)){
		cout<<"recv error in dstport!"<<endl;
		return;                                                                                                             
	}
	cout<<"web ip: "<<inet_ntoa(remote.sin_addr)<<" web port: "<<ntohs(remote.sin_port)<<endl;
	//至此获得客户端想要连接服务器的ip,port
	// VER   REP   RSV   ATYP   BND.ADDR   BND.PORT 
	//  1     1   X’00’   1     Variable    2 
	addrRep repaddr;
	repaddr.ver=5;
	repaddr.rsv=0;//标识为RSV 的字段必须设为X’00’ 
	int out=socket(AF_INET,SOCK_STREAM,0);
	socklen_t rlen=sizeof(remote);
	if(0!=connect(out,(sockaddr*)&remote,rlen)){
		perror("connect");
		repaddr.rep=3;//网络不可达 
		if(0!=send_data(in,&repaddr,sizeof(repaddr))){
			cout<<"error in connect remote!"<<endl;
			return;
		}
	}
	cout<<"connect success!"<<endl;
	repaddr.rep=0;//成功
	repaddr.atpy=1;//ipv4
	if(0!=send_data(in,&repaddr,sizeof(repaddr))){
		cout<<"send error to client!"<<endl;
		return;
	}
	struct sockaddr_in local;
	socklen_t locallen=sizeof(local);
	if(0!=getsockname(out,(sockaddr*)&local,&locallen)){
		perror("getsockname");
		return;                                     
	}                                                                                                                   
	cout<<"ip: "<<inet_ntoa(local.sin_addr)<<" port: "<<ntohs(local.sin_port)<<endl;
	//sleep(5);
	int len=sizeof(local.sin_addr)+sizeof(local.sin_port);
	if(0!=send_data(in,&local,len)){
		cout<<"send error to client!"<<endl;
		return;
	}
	cout<<"协商完成"<<endl;
	///////////////////////////////////////////////////////////////////
	////至此，完成了协商过程，                                                                                                                                                            
	////以下为正式处理HTTP请求
	pthread_t th1,th2;

	ThreadInfo thInfo1={in,out,th2};                                                                                        
	ThreadInfo thInfo2={out,in,th1};

	pthread_create(&th1,NULL,thre_handler,&thInfo1);
	pthread_create(&th2,NULL,thre_handler,&thInfo2);   
}                                                                                                                                                                                                            
void* thre_handler(void* argc)
{   
	ThreadInfo* info=(ThreadInfo*)argc;
	int in=info->in;
	int out=info->out;
	pthread_t id=info->id;
	char buf[SIZE*SIZE];
	while(1){
		int ret=0;                                                                                                          
		ret=recv(in,buf,sizeof(buf),0);
		if(ret<=0){
			perror("recv");
			break;
		}
		ret=send(out,buf,ret,0);
	}
	close(in);
	close(out);
	pthread_cancel(id);                                                                                                     
	return NULL;
}
