/*************************************************************************
	> File Name: protocol.h
	> Author: chehlling
	> Mail: chehlling@126.com 
	> Created Time: Sun 13 Nov 2016 11:13:24 PM PST
 ************************************************************************/

#pragma once
#pragma pack(1)
  
struct startReq//请求
{
	    char ver;//5
		    char n_methods;//方法数量
};
                                                                                                                                       
struct startRep//回复
{
	    char ver;//5
		    char method;
};
  
// VER    CMD    RSV    ATYP    DST.ADDR    DST.PROT 
// //  1      1    X’00’    1      Variable       2
struct addrReq
{   
    char ver;
    char cmd;//CONNECT：X’01'    BIND：X’02’   UDP ASSOCIATE：X’03’
    char rsv;//保留
    char atyp;//后面的地址类型
};  
         
// VER   REP   RSV   ATYP   BND.ADDR   BND.PORT 
//  1     1   X’00’   1     Variable     2 
struct addrRep
{   
    char ver;
    char rep;                                                              
    char rsv;
    char atpy;
};
