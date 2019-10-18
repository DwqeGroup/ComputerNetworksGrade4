
#pragma once

 

//服务器侦听控制连接请求的端口

#define CMD_PORT 5858

//客户机侦听数据连接请求的端口

#define DATA_PORT 5850

//命令报文参数缓存的大小

#define CMD_PARAM_SIZE 256

//回复报文消息缓存的大小

#define RSPNS_TEXT_SIZE 256

#define BACKLOG 10

#define DATA_BUFSIZE 4096

 

//命令类型

typedef enum {

	LS, PWD, CD, DOWN, UP, QUIT

} CmdID;

 

//命令报文,从客户端发往服务器

typedef struct _CmdPacket {

	CmdID cmdid;

	char param[CMD_PARAM_SIZE];

} CmdPacket;

 

//回复报文的类型

typedef enum {

	OK, ERR

} RspnsID;

 

//回复报文,从服务器发往客户端

typedef struct _RspnsPacket {

	RspnsID rspnsid;

	char text[RSPNS_TEXT_SIZE];

} RspnsPacket;

