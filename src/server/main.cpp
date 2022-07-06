#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>
using namespace std;


//处理服务器异常退出：将登录状态重置
void resetstatehandler(int)
{
    Chatservice::instance()->resetstate();
    exit(0);
}


int main(int argc, char **argv){

    if(argc < 3)
    {
        cerr << "Command invalid. Please input: ./chatserver 127.0.0.1 6000 or ./chatserver 127.0.0.1 6002" << endl;
        exit(-1);
    }

    //解析传入参数的IP and PORT
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //捕获信号处理服务器异常退出
    signal(SIGINT, resetstatehandler);

    EventLoop loop;
    InetAddress addr(ip, port);

    ChatServer server(&loop, addr, "zjp-chatserver"); 
    server.start(); //启动服务
    loop.loop(); //事件循环

    return 0;
}