#pragma once 

#include "ThreadPool.hpp"

#define DEFPORT 8080 //默认端口号

class HttpServer
{
private:
    int port;
    int lsock;

    static pthread_mutex_t lock;
    static HttpServer* hp;

    ThreadPool* tp;
private:
    //单例模式
    HttpServer(int _port)
      :port(_port),lsock(-1)
    {}

    HttpServer(HttpServer&) = delete;
public:
    static HttpServer* GetInstance(int _port)
    {
        if(hp == nullptr)
        {
            //加锁
            pthread_mutex_lock(&lock); 
            //双重判断，确保安全
            //有可能一个两个线程同时进入最外边的判断
            //但是只有一个线程能够申请到所
            //因此，申请到锁后还要判断是否有其他线程进入
            if(hp == nullptr)
               hp = new HttpServer(_port);
            pthread_mutex_unlock(&lock);
        }
        return hp;
    }
    //初始化--创建套接字相关
    void Init()
    {
        //初始化状态码
        HttpReponse::InitStatusCodeMap(); 
        //初始化文件类型表
        HttpReponse::InitContenTypeMap();

        //创建套接字
        lsock = Sock::Socket();
        //设置端口复用
        Sock::Setsockopt(lsock);
      
        //绑定
        Sock::Bind(lsock,port);
        //监听
        Sock::Listen(lsock);

        tp = new ThreadPool();
        tp->InitThreadPool();
    }
    
    void Start()
    {
        for(;;)
        {
            int sock = Sock::Accept(lsock);
            if(sock < 0)
                continue;
            //接收成功
            LOG(Notice,"get a new linke");
            //创建线程，处理请求
            pthread_t tid;

            int* sockp = new int(sock);
            pthread_create(&tid,nullptr,Entry::HandlerHttp,sockp);
            pthread_detach(tid);
            /*Task* tk = new Task(sock);
            tp->PushTask(tk);*/
        }
    }

    ~HttpServer()
    {
        //关闭打开的文件描述符
        if(lsock >= 0)  
            close(lsock);
    }
};

HttpServer* HttpServer::hp = nullptr;
pthread_mutex_t HttpServer::lock = PTHREAD_MUTEX_INITIALIZER;
