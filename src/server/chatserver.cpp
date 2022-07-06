#include <functional>
#include <string>
#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"

using namespace std;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
                       : _server(loop, listenAddr, nameArg), _loop(loop)
{
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    _server.setThreadNum(4);
}

void ChatServer::start()
{
    _server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr &con)
{
    if(!con->connected()) {
        //处理用户连接异常断开（退出）
        Chatservice::instance()->clientCloseException(con);
        con->shutdown();
    }
}
void ChatServer::onMessage(const TcpConnectionPtr &con,
                           Buffer *buff,
                           Timestamp ts)
{
    string buf = buff->retrieveAllAsString();
    json js = json::parse(buf);
    //网络模块 和 业务模块 解耦
    //回调对应的处理业务(获取对应的业务处理器，根据json中的消息执行回调函数)
    Chatservice::instance()->getHandler(js["msgid"].get<int>()) (con, js, ts);
}