#ifndef _CHATSERVICE_H_
#define _CHATSERVICE_H_
#include "json.hpp"
#include <functional>
#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <mutex>
#include "usermodel.hpp"
#include "offlinemsgmodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

using msgHandler = std::function<void(const TcpConnectionPtr &con, json &js, Timestamp)>;

// 聊天服务器业务类
class Chatservice
{
public:
    static Chatservice* instance();

    void login(const TcpConnectionPtr& con, json& js, Timestamp ts);

    void reg(const TcpConnectionPtr&, json& js, Timestamp ts);

    //返回事件处理器
    msgHandler getHandler(int msgid);

    //处理用户连接异常断开
    void clientCloseException(const TcpConnectionPtr& con);

    //处理一对一聊天业务
    void oneChat(const TcpConnectionPtr&, json& js, Timestamp ts);

    //重置状态信息
    void resetstate();

    //添加好友
    void add(const TcpConnectionPtr&, json& js, Timestamp ts);

    //创建群组业务
    void createGroup(const TcpConnectionPtr&, json& js, Timestamp ts);

    //加入群组业务
    void addGroup(const TcpConnectionPtr&, json& js, Timestamp ts);

    //群组聊天
    void groupChat(const TcpConnectionPtr&, json& js, Timestamp ts);

    //处理注销业务
    void loginout(const TcpConnectionPtr &con, json &js, Timestamp ts);

    //从redis消息队列中获取消息
    void handlerRedisSubscribeMessage(int, string);

private:
    Chatservice();

    //存储消息ID和对应的业务处理方法
    unordered_map<int, msgHandler> _msghandler;

    //用户数据处理业务对象
    UserModel _usermodel;

    //存储用户连接的容器  key:用户id  value：tcp连接句柄
    unordered_map<int, TcpConnectionPtr> _userconnection;

    mutex _connMutex;

    //操作离线消息
    OfflineMsgModel _offlinemsgmodel;

    //操作添加查询好友
    FriendModel _friendmodel;

    //操作群组
    GroupModel _groupmodel;

    //redis发布订阅--跨服务器通信操作
    Redis _redis;
};

#endif