#pragma once
#include "Sock.hpp"

#include "Protocol.hpp"
#include <pthread.h>
#include <queue>

class Task
{
private:
    int sock;
public:
    Task(int sk):sock(sk)
    {}

    void Run()
    {
        Entry::HandlerHttp(sock);
    }
};

class ThreadPool
{
private:
    std::queue<Task*> qu;//任务队列

    pthread_mutex_t mut;//互斥量
    pthread_cond_t cond;//条件变量

    int MaxNum;//最大线程数量
public:
    ThreadPool(int n = 2):MaxNum(n)
    {}
    
    static void* Routinue(void* argc)
    {
        ThreadPool* tp_this = (ThreadPool*)argc;
        
        pthread_mutex_lock(&tp_this->mut);
        //防止挂起失败，因此while循环
        while(tp_this->qu.empty())
        {
            //等待push后的信号
            pthread_cond_wait((&tp_this->cond),&(tp_this->mut));
        }
        //从任务队列中取任务执行
        Task* tk = tp_this->PopTask();
        pthread_mutex_unlock(&tp_this->mut);
        LOG(Notice,"run"); 
        tk->Run();

        //this指针会在对象调用结束后自动销毁
        delete tk;
        return nullptr;
    }

    void InitThreadPool()
    {
        //初始化互斥锁和条件变量
        pthread_mutex_init(&mut,nullptr);
        pthread_cond_init(&cond,nullptr);
        
        //创建线程，并进行线程分离
        pthread_t pid;
        for(auto i = 1;i <= MaxNum;i++)
        {
            pthread_create(&pid,nullptr,Routinue,this);
            pthread_detach(pid);
        }
    }
    void PushTask(Task* tk)
    {
        pthread_mutex_lock(&mut);
        qu.push(tk);
        pthread_mutex_unlock(&mut);

        //条件满足，通知线程从任务队列读取数据
        pthread_cond_signal(&cond);
    }
    Task* PopTask()
    {
        Task* tk = qu.front();

        qu.pop();
        return tk;
    }
    ~ThreadPool()
    {
        pthread_mutex_destroy(&mut);
        pthread_cond_destroy(&cond);
    }
};
