#include "Sock.hpp"
#include <sys/time.h>

#define DFL_PORT 8080
#define MAX_FD_SET sizeof(fd_set)

class Select
{
private:
    int lsock;
    int port; 
    int fd_arry[MAX_FD_SET] = {-1};
public:
    Select(int _port = DFL_PORT):port(_port)
    {}

    void InitServer()
    {
        lsock = Sock::Socket();
        Sock::SetSockOpt(lsock);
        Sock::Bind(lsock,port);
        Sock::Listen(lsock);

        //一开始只有一个文件描述符：将lsock加入到fd_array中
        fd_arry[0] = lsock;
    }
     void AddFd2Array(int sock)
     {
          size_t i = 0;
          for( ; i < MAX_FD_SET; i++ )
          {
              if(fd_arry[i] == -1)
              {
                  break;
              }
          }
          if(i >= MAX_FD_SET)
          {
              //fd_Set设置满了
              cerr << "fd array is full, close sock" << endl;
              close(sock);
          }
          else
          {
              fd_arry[i] = sock;
              cout << "fd: " << sock << " add to select ..." << endl;
          }
     }

    void DefFdFromArray(size_t index)
    {
        if(index >=0 && index < MAX_FD_SET)
        {
            fd_arry[index] = -1;
        }
    }
    void HandlerEvent(fd_set* rfd)
    {
        //遍历rfd
        for(size_t i = 0;i < MAX_FD_SET;i++)
        {
           if(fd_arry[i] == -1)
              continue;
          
           if(FD_ISSET(fd_arry[i],rfd))
           {
              if(fd_arry[i] == lsock)
              {
                  int sk = Sock::Accept(lsock);
                  if(sk > 0)
                  {
                      cout<<"get a new link..."<<endl;
                      AddFd2Array(sk); 
                  }
              }
              else 
              {
                  char buf[10240];
                  ssize_t s= recv(fd_arry[i], buf, sizeof(buf), 0);
                  if(s>0)
                  {
                      buf[s] = 0;
                      cout << "client# " << buf << endl;
                  }
                  else if(s == 0)
                  {
                      cout << "clien quit" << endl;
                      close(fd_arry[i]);
                      DefFdFromArray(i);
                  }
              }
           }
        }
    }

    void SatrtServer()
    {
        int maxfd = -1;
        for(;;)
        {
            fd_set rfd;
            FD_ZERO(&rfd); 
            //将fd_srray设置仅rfd
            for(size_t i = 0;i < MAX_FD_SET;i++)
            {
                if(fd_arry[i] > maxfd)
                    maxfd = fd_arry[i];
                FD_SET(fd_arry[i],&rfd);
            }

            switch(select(maxfd+1,&rfd,nullptr,nullptr,nullptr))
            {
                case 0:
                  cout<<"timeout ..."<<endl;
                  break;
                case -1:
                  cout<<"wait error..."<<endl;
                  break;
                default:
                  HandlerEvent(&rfd);
                  break;
            }
        }
    }
};
