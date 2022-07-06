#include "redis.hpp"
#include <iostream>
#include <thread>

Redis::Redis() : _pubilsh_context(nullptr), _subscribe_context(nullptr)
{
}

Redis::~Redis()
{
    if (_pubilsh_context)
    {
        redisFree(_pubilsh_context);
    }

    if (_subscribe_context)
    {
        redisFree(_subscribe_context);
    }
}

//连接redis服务器
bool Redis::connect()
{
    //发布消息的上下文连接
    _pubilsh_context = redisConnect("127.0.0.1", 6379);
    if (!_pubilsh_context)
    {
        cerr << "connect redis failed." << endl;
        return false;
    }

    //订阅消息的上下文连接
    _subscribe_context = redisConnect("127.0.0.1", 6379);
    if (!_subscribe_context)
    {
        cerr << "connect redis failed." << endl;
        return false;
    }

    //在单独线程中监听通道上的事件，有事件则上报业务层
    thread t([&]()
             { observer_channel_message(); });
    t.detach();

    cout << "connect redis-server success!" << endl;

    return true;
}

//向redis指定通道发送消息
bool Redis::pubilsh(int channel, string message)
{
    redisReply *reply = (redisReply *)redisCommand(_pubilsh_context, "PUBLISH %d %s", channel, message.c_str());
    if (!reply)
    {
        cerr << "publish command failed!" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

//向redis指定通道订阅消息
bool Redis::subscribe(int channel)
{
    // SUBSCRIBE本身阻塞，会和单独开辟的线程抢占资源（接收消息在专门线程中执行）
    //此函数只做消息订阅
    if (REDIS_ERR == redisAppendCommand(this->_subscribe_context, "SUBSCRIBE %d", channel))
    {
        cerr << "subscribe command failed!" << endl;
        return false;
    }

    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subscribe_context, &done))
        {
            cerr << "subscribe command failed!" << endl;
            return false;
        }
    }

    return true;
}

//取消订阅消息
bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(this->_subscribe_context, "UNSUBSCRIBE %d", channel))
    {
        cerr << "unsubscribe command failed!" << endl;
        return false;
    }

    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subscribe_context, &done))
        {
            cerr << "unsubscribe command failed!" << endl;
            return false;
        }
    }
    return true;
}

//开辟独立线程，接收通道消息
void Redis::observer_channel_message()
{
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(this->_subscribe_context, (void **)&reply))
    {
        //订阅收到的消息：三元素数据
        if (reply && reply->element[2] && reply->element[2]->str)
        {
            //存在：给上层业务通报
            _notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
        }

        freeReplyObject(reply);
    }

    cerr << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<" << endl;
}

//初始化向上层业务通报通道消息的回调函数
void Redis::init_notify_handler(function<void(int, string)> func)
{
    this->_notify_message_handler = func;
}