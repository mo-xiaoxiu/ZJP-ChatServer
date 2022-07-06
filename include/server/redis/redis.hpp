#ifndef _REDIS_H_
#define _REDIS_H_

#include <hiredis/hiredis.h>
#include <functional>
#include <string.h>
using namespace std;


class Redis
{
public:
    Redis();
    ~Redis();

    //连接redis服务器
    bool connect();

    //向redis指定通道发送消息
    bool pubilsh(int channel, string message);

    //向redis指定通道订阅消息
    bool subscribe(int channel);

    //取消订阅消息
    bool unsubscribe(int channel);

    //开辟独立线程，接收通道消息
    void observer_channel_message();

    //初始化向上层业务通报通道消息的回调函数
    void init_notify_handler(function<void(int, string)> func);

private:
    //上下文信息
    //publish上下文信息：在发送发布-订阅命令时，需要在不同的上下文Context环境中进行
    redisContext* _pubilsh_context;
    redisContext* _subscribe_context;

    //回调：收到订阅消息，向server上报
    function<void(int, string)> _notify_message_handler;
};

#endif