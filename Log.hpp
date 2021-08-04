#pragma once 

#include <iostream>
#include <sys/time.h>

//__FILE__表示获取文件名，__LINE__获取调用行
//#表示将后边的内容转换成字符串
#define LOG(level,message) \
        Log(#level,message,__FILE__,__LINE__)

//日志等级
#define Notice  1  //正常信息
#define Warning 2  //警告，不影响程序正常运行
#define Error   3  //错误，没必要终止进程
#define Fatal   4  //严重问题，直接退出进程

enum
{
    SocketErr=1,
    BindErr,
    ListenErr,
    //...
};

void Log(std::string level,std::string message,std::string filename,size_t line)
{
    //gettimeofday函数获取当前时间对应的时间戳
    struct timeval time;
    gettimeofday(&time,nullptr);
    //打印日志信息---可以替换成输出到文件
    std::cout<<"["<<level<<"] "<<"["<<message<<"] "<<"["<<time.tv_sec<<"] "<<"["<<filename<<"] "<<"["<<line<<"]"<<std::endl;
}


