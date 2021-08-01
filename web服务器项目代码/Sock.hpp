#pragma once 

/*创建套接字相关接口*/

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <vector>
#include <unordered_map>
#include <sstream> 
#include "Log.hpp"

#define MAXLISTEN 5

class Sock
{
public:
    //创建套接字
    static int Socket()
    {
        int lsock = socket(AF_INET,SOCK_STREAM,0);

        //创建失败--退出进程
        if(lsock < 0)
        {
            //生成日志
            LOG(Fatal,"soecket error"); 
            //退出进程
            exit(1);
        }

        return lsock;
    }
    //将端口号port绑定到sock中
    static void Bind(int sock,int port)
    {
        struct sockaddr_in local;
        bzero(&local,sizeof(local));

        local.sin_family = AF_INET;
        local.sin_port = htons(port);
        local.sin_addr.s_addr = htonl(INADDR_ANY);

        //绑定
        if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
        {
            //绑定失败
            LOG(Fatal,"Bind error");
            exit(BindErr);
        }
    }
    //监听
    static void Listen(int sock)
    {
        int s = listen(sock,MAXLISTEN);

        if(s < 0)
        {
            LOG(Fatal,"listen error");
            exit(ListenErr);
        }
    }

    static void Setsockopt(int sock)
    {
        int opt = 1;
        setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    }

    static int Accept(int sock)
    {
        struct sockaddr_in rem;
        socklen_t len = sizeof(rem);
        int s = accept(sock,(struct sockaddr*)&rem,&len);

        if(s < 0)
        {
            LOG(Warning,"accept error");
        }

        return s;
    }
    static void GetLine(int sock,std::string &str)
    {
        //行结束--- \n \r \r\n分情况处理
        char ch = 'X';

        while(ch != '\n')
        { 
            //从sock中一次读一个字符
            //读取成功，走下面逻辑
            //读取失败，继续读取
            if(recv(sock,&ch,1,0) > 0 )
            {
                //将\r或\r\n替换成\n
                if(ch == '\r')
                {
                    //检测下一个是否是\n
                    //MSG_PEEK选项的意思是将内核中的数据拷贝一份，并不会从缓冲区中删除
                    //也就是说，下面这个recv它并不会把读到的字符从缓冲区中删除
                    //起到了检测的作用
                    if(recv(sock,&ch,1,MSG_PEEK) > 0)
                    {
                        //如果是\n，则读取出来
                        if(ch == '\n')
                            recv(sock,&ch,1,0);
                    }
                    //将换行符统一成'\n'
                    ch = '\n';
                }

                //push字符
                if(ch != '\n')
                    str += ch;
           }
        }
    }    
};

