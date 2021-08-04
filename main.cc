#include "HttpServer.hpp"
#include "Log.hpp"

int main(int argc,char* argv[])
{
    int port = 8080;

    if(argc == 2)
        port = atoi(argv[1]);

    HttpServer* hp = HttpServer::GetInstance(port);
    hp->Init();
    hp->Start();

    return 0;
}
