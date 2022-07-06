#include "chatservice.hpp"
#include "public.hpp"
#include <functional>
#include <vector>
#include <muduo/base/Logging.h>

Chatservice::Chatservice() //初始化消息处理器
{
    _msghandler.insert({LOGIN_MSG, std::bind(&Chatservice::login, this, _1, _2, _3)});

    _msghandler.insert({LOGINOUT_MSG, std::bind(&Chatservice::loginout, this, _1, _2, _3)});

    _msghandler.insert({REG_MSG, std::bind(&Chatservice::reg, this, _1, _2, _3)});

    _msghandler.insert({ONE_CHAT_MSG, std::bind(&Chatservice::oneChat, this, _1, _2, _3)});

    _msghandler.insert({ADD_FRIEND_MSG, std::bind(&Chatservice::add, this, _1, _2, _3)});

    //群组聊天业务相关的回调处理器注册
    _msghandler.insert({CREATE_CROUP_MSG, std::bind(&Chatservice::createGroup, this, _1, _2, _3)});
    _msghandler.insert({ADD_GROUP_MSG, std::bind(&Chatservice::addGroup, this, _1, _2, _3)});
    _msghandler.insert({GROUP_CHAT_MSG, std::bind(&Chatservice::groupChat, this, _1, _2, _3)});

    //连接redis
    if(_redis.connect())
    {
        _redis.init_notify_handler(std::bind(&Chatservice::handlerRedisSubscribeMessage, this, _1, _2));
    }
}

void Chatservice::resetstate()
{
    _usermodel.resetState();
}

Chatservice *Chatservice::instance() //单例  返回唯一实例
{
    static Chatservice service;
    return &service;
}

msgHandler Chatservice::getHandler(int msgid)
{
    auto it = _msghandler.find(msgid);
    if (it == _msghandler.end())
    {
        return [=](const TcpConnectionPtr &con, json &js, Timestamp ts)
        {
            LOG_ERROR << "msgid: " << msgid << "  can't find handler!!!";
        };
    }
    else
    {
        return _msghandler[msgid];
    }
}

//处理注销业务
void Chatservice::loginout(const TcpConnectionPtr &con, json &js, Timestamp ts)
{
    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userconnection.find(userid);
        if(it != _userconnection.end())
        {
            _userconnection.erase(it);
        }
    }

    //注销：下线，redis中取消订阅
    _redis.unsubscribe(userid);

    //更新用户状态信息
    User user(userid, "", "", "offline");
    _usermodel.updateState(user);
}


void Chatservice::login(const TcpConnectionPtr &con, json &js, Timestamp ts)
{
    // for debug
    // LOG_INFO << "call login handler...";

    int id = js["id"].get<int>();
    string pwd = js["password"];

    User user = _usermodel.query(id);
    if (user.getId() == id && user.getPwd() == pwd)
    {
        //用户已经登录
        if (user.getState() == "online")
        {
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "This user is already online, please input a new userid.";
            con->send(response.dump());
        }
        else
        {
            //登录成功，将用户连接储存起来：需要注意线程安全问题
            {
                lock_guard<mutex> lk(_connMutex);
                _userconnection.insert({id, con});
            }

            //用户登陆成功，将该id作为channel订阅消息
            _redis.subscribe(id);

            //登录成功，用户状态更新
            user.setState("online");
            _usermodel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = id;
            response["name"] = user.getName();

            //查看是否有离线消息
            vector<string> vec = _offlinemsgmodel.queryofflinemsg(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                _offlinemsgmodel.removeofflinemsg(id);
            }

            //查询好友信息并返回
            vector<User> uservec = _friendmodel.query(id);
            if (!uservec.empty())
            {
                vector<string> vec2;
                for (auto &user : uservec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            //查询群组消息并返回
            vector<Group> groupuservec = _groupmodel.queryGroups(id);
            if(!groupuservec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for(Group& group: groupuservec) //获取所在组
                {
                    json groupJs;
                    groupJs["id"] = group.getId();
                    groupJs["groupname"] = group.getName();
                    groupJs["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for(auto& user: group.getUsers()) //获取所在组的所有组成员
                    {
                        json groupusers;
                        groupusers["id"] = user.getId();
                        groupusers["name"] = user.getName();
                        groupusers["state"] = user.getState();
                        groupusers["role"] = user.getRole();
                        userV.push_back(groupusers.dump());
                    }
                    groupJs["users"] = userV;
                    groupV.push_back(groupJs.dump());
                }

                response["groups"] = groupV;
            }

            con->send(response.dump());
        }
    }
    else //用户不存在或者密码错误
    {
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "User non-existent or password is wrong.";
        con->send(response.dump());
    }
}

void Chatservice::reg(const TcpConnectionPtr &con, json &js, Timestamp)
{
    // for debug
    // LOG_INFO << "call reg handler...";
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);

    if (_usermodel.insert(user))
    {
        //插入成功：注册成功，返回响应消息  REG_MSG_ACK
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0; //错误码为0
        response["id"] = user.getId();

        con->send(response.dump());
    }
    else
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1; // 1 表示错误
        // response["errmsg"] = "register failed.";

        con->send(response.dump());
    }
}

//处理用户连接异常断开
void Chatservice::clientCloseException(const TcpConnectionPtr &con)
{
    User user;
    //查找用户连接，并删除，注意线程安全问题
    {
        lock_guard<mutex> lk(_connMutex);
        for (auto it = _userconnection.begin(); it != _userconnection.end(); it++)
        {
            if (it->second == con)
            {
                user.setId(it->first);
                _userconnection.erase(it);
                break;
            }
        }
    }

    //用户异常退出：下线，取消订阅
    _redis.unsubscribe(user.getId());

    //修改用户连接的登录状态
    if (user.getId() != -1)
    {
        user.setState("offline");
        _usermodel.updateState(user);
    }
}

//处理一对一聊天业务
void Chatservice::oneChat(const TcpConnectionPtr &, json &js, Timestamp ts)
{
    int toid = js["toid"].get<int>(); //聊天对端发来的toid
    {
        lock_guard<mutex> lk(_connMutex);
        auto it = _userconnection.find(toid);
        if (it != _userconnection.end())
        {
            // toid在线 转发消息
            it->second->send(js.dump());
            return;
        }
    }

    //查询toid是否在线
    User user = _usermodel.query(toid);
    if(user.getState() == "online")
    {
        //在本地服务器的用户连接集中找不到该连接，经过数据库查询该用户为在线状态，则说明对端用户在另一台服务器登录，向该通道发布消息
        _redis.pubilsh(toid, js.dump());
        return;
    }

    // toid不在线：存储离线消息
    _offlinemsgmodel.insertofflinemsg(toid, js.dump());
}

//添加好友
void Chatservice::add(const TcpConnectionPtr &, json &js, Timestamp ts)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    //存储好友消息
    _friendmodel.insert(userid, friendid);
}

//创建群组业务
void Chatservice::createGroup(const TcpConnectionPtr &, json &js, Timestamp ts)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    //存储新创建的群组信息
    Group group(-1, name, desc);
    if(_groupmodel.createGroup(group))
    {
        //存储群组创建人信息
        _groupmodel.addGroup(userid, group.getId(), "creator");
    }
}

//加入群组业务
void Chatservice::addGroup(const TcpConnectionPtr &, json &js, Timestamp ts)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupmodel.addGroup(userid, groupid, "normal");
}

//群组聊天
void Chatservice::groupChat(const TcpConnectionPtr &, json &js, Timestamp ts)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridvec = _groupmodel.queryGroupUsers(userid, groupid);

    for(int id: useridvec)
    {
        auto it = _userconnection.find(id);
        if(it != _userconnection.end())
        {
            //转发群消息
            it->second->send(js.dump());
        }
        else
        {
            //查询toid是否在线
            User user = _usermodel.query(id);
            if(user.getState() == "online")
            {
                //情况同onechat一致
                _redis.pubilsh(id, js.dump());
            }
            else
            {
                //存储离线消息
                _offlinemsgmodel.insertofflinemsg(id, js.dump());
            }
        }
    }
}

//从redis消息队列中获取消息
void Chatservice::handlerRedisSubscribeMessage(int userid, string message)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userconnection.find(userid);
    if(it != _userconnection.end())
    {
        it->second->send(message);
    }

    //对端发布消息之后本端对应用户下线，存储该用户的离线消息
    _offlinemsgmodel.insertofflinemsg(userid, message);
}