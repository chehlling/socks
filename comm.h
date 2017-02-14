/*************************************************************************
	> File Name: comm.h
	> Author: chehlling
	> Mail: chehlling@126.com 
	> Created Time: Sun 13 Nov 2016 11:15:49 PM PST
 ************************************************************************/

#pragma once
  
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
  
#define SIZE 1024
  
int recv_data(int sock,void* buf,size_t size);
int send_data(int sock,void* buf,size_t size);
