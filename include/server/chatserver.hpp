#ifndef _CHATSERVER_H_
#define _CHATSERVER_H_
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <string>


using namespace muduo;
using namespace muduo::net;
using namespace std;

class ChatServer
{
public:
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const string &nameArg);

    void start();
private:
    void onConnection(const TcpConnectionPtr &con);
    void onMessage(const TcpConnectionPtr &con,
                   Buffer *buff,
                   Timestamp ts);

    TcpServer _server;
    EventLoop *_loop;
};

#endif