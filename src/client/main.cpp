#include "json.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono> //系统时间
#include <ctime>
#include <unordered_map>
#include <functional>
using namespace std;
using json = nlohmann::json;

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <atomic>

#include "group.hpp"
#include "user.hpp"
#include "public.hpp"

// 控制主菜单页面程序：注销之后回到主菜单页面
bool isMainMenuRunning = false;

// 用于读写线程之间的通信
sem_t rwsem;
// 记录登录状态
atomic_bool g_isLoginSuccess{false};

// 记录当前系统登录的用户信息
User g_currentUser;
// 记录当前登录用户的好友列表信息
vector<User> g_currentUserFriendList;
// 记录当前登录用户的群组成员列表信息
vector<Group> g_currentUserGroupList;

// 接收线程
void readTaskHandler(int clientfd);
// 获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime();
// 主聊天页面程序
void mainMenu(int);
// 显示当前登录成功用户的基本信息
void showCurrentUserData();

//聊天客户端：main用于发送线程，子线程用于接收线程
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatClient 127.0.0.1 6000" << endl;
        exit(-1);
    }

    //解析传参：IP and PORT
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //创建客户端socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        cerr << "socket create error" << endl;
        exit(-1);
    }

    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(port);

    //客户端连接服务端
    if (-1 == connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)))
    {
        cerr << "connect server errno." << endl;
        close(clientfd);
        exit(-1);
    }

    static int threadnum = 0;
    if (threadnum == 0)
    {
        std::thread readTask(readTaskHandler, clientfd);
        readTask.detach();
        threadnum++;
    }

    //主线程用于接收用户输入，负责接收子线程
    for (;;)
    {
        // 显示首页面菜单 登录、注册、退出
        cout << "========================" << endl;
        cout << "1. login" << endl;
        cout << "2. register" << endl;
        cout << "3. quit" << endl;
        cout << "========================" << endl;
        cout << "choice:";
        int choice = 0;
        cin >> choice;
        cin.get(); // 读掉缓冲区残留的回车

        switch (choice)
        {
        case 1 /* 登录 */:
        {
            int id = 0;
            char pwd[50] = {0};
            cout << "userid:";
            cin >> id;
            cin.get(); // 读掉缓冲区残留的回车
            cout << "userpassword:";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            string request = js.dump();

            g_isLoginSuccess = false;

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send login msg errno: " << request << endl;
            }

            sem_wait(&rwsem); //子线程处理完登录响应消息，通知这里

            //登录成功，进入聊天主界面
            if (g_isLoginSuccess)
            {
                isMainMenuRunning = true;
                mainMenu(clientfd);
            }
        } /* code */
        break;
        case 2: //注册
        {
            char name[50] = {0};
            char pwd[50] = {0};
            cout << "username:";
            cin.getline(name, 50);
            cout << "userpassword:";
            cin.getline(pwd, 50); //使用getline读取一整行输入

            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send reg msg error:" << request << endl;
            }

            sem_wait(&rwsem); // 等待信号量，子线程处理完注册消息会通知
        }
        break;
        case 3: //退出服务
        {
            close(clientfd);
            sem_destroy(&rwsem);
            exit(0);
        }
        break;
        default:
            cerr << "invalid input." << endl;
            break;
        }
    }
    return 0;
}

//处理注册响应
void doRegResponse(json &responseJs)
{
    //注册失败
    if (0 != responseJs["errno"].get<int>())
    {
        cerr << "name is already exist, register error!" << endl;
    }
    else //注册成功
    {
        cout << "name register success, userid is " << responseJs["id"]
             << ", do not forget it!" << endl;
    }
}

//处理登录响应
void doLoginResponse(json &responseJs)
{
    //登录失败
    if (0 != responseJs["errno"].get<int>())
    {
        cerr << responseJs["errmsg"] << endl;
        g_isLoginSuccess = false;
    }
    else //登录成功
    {
        //记录当前用户id and name
        g_currentUser.setId(responseJs["id"]);
        g_currentUser.setName(responseJs["name"]);

        //记录当前用户的好友列表
        if (responseJs.contains("friends"))
        {
            //初始化
            g_currentUserFriendList.clear();

            vector<string> vec = responseJs["friends"];
            for (auto &sv : vec)
            {
                json js = json::parse(sv);
                User user;
                user.setId(js["id"]);
                user.setName(js["name"]);
                user.setState(js["state"]);
                g_currentUserFriendList.push_back(user);
            }
        }

        //记录当前用户的群组列表信息
        if (responseJs.contains("groups"))
        {
            //初始化：防止重复拉入数据
            g_currentUserGroupList.clear();

            vector<string> vec_1 = responseJs["groups"];
            for (auto &sv : vec_1)
            {
                json js = json::parse(sv);
                Group group;
                group.setId(js["id"]);
                group.setName(js["groupname"]);
                group.setDesc(js["groupdesc"]);

                vector<string> vec_2 = js["users"];
                for (auto &sv : vec_2)
                {
                    GroupUser user;
                    json js = json::parse(sv);
                    user.setId(js["id"]);
                    user.setName(js["name"]);
                    user.setState(js["state"]);
                    user.setRole(js["role"]);
                    group.getUsers().push_back(user);
                }

                g_currentUserGroupList.push_back(group);
            }
        }

        //显示登录用户的基本信息
        showCurrentUserData();

        //显示当前用户的离线消息：个人聊天 or 群组聊天
        if (responseJs.contains("offlinemsg"))
        {
            vector<string> vec = responseJs["offlinemsg"];
            for (auto &str : vec)
            {
                json js = json::parse(str);
                // time + [id] + name + " said: " + xxx
                if (ONE_CHAT_MSG == js["msgid"])
                {
                    cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                         << " said: " << js["msg"].get<string>() << endl;
                }
                else
                {
                    cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                         << " said: " << js["msg"].get<string>() << endl;
                }
            }
        }

        g_isLoginSuccess = true;
    }
}

//子线程：接收线程
void readTaskHandler(int clientfd)
{
    for (;;)
    {
        char buff[1024] = {0};
        int len = recv(clientfd, buff, 1024, 0); //阻塞
        if (-1 == len || len == 0)
        {
            close(clientfd);
            exit(-1);
        }

        //接收chatserver发送过来的数据， 反序列化封装json对象
        json js = json::parse(buff);
        int msgtype = js["msgid"];
        if (ONE_CHAT_MSG == msgtype) //一对一聊天
        {
            cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                 << " said: " << js["msg"].get<string>() << endl;
            continue;
        }

        if (GROUP_CHAT_MSG == msgtype) //群聊消息
        {
            cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                 << " said: " << js["msg"].get<string>() << endl;
            continue;
        }

        if (LOGIN_MSG_ACK == msgtype) //登录
        {
            doLoginResponse(js); // 处理登录响应的业务逻辑
            sem_post(&rwsem);    // 通知主线程，登录结果处理完成
            continue;
        }

        if (REG_MSG_ACK == msgtype) //注册
        {
            doRegResponse(js);
            sem_post(&rwsem); // 通知主线程，注册结果处理完成
            continue;
        }
    }
}

// 显示当前登录成功用户的基本信息
void showCurrentUserData()
{
    cout << "======================login user======================" << endl;
    cout << "current login user => id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << endl;
    cout << "----------------------friend list---------------------" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (User &user : g_currentUserFriendList)
        {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "----------------------group list----------------------" << endl;
    if (!g_currentUserGroupList.empty())
    {
        for (Group &group : g_currentUserGroupList)
        {
            cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
            for (GroupUser &user : group.getUsers())
            {
                cout << user.getId() << " " << user.getName() << " " << user.getState()
                     << " " << user.getRole() << endl;
            }
        }
    }
    cout << "======================================================" << endl;
}

// "help" command handler
void help(int fd = 0, string str = "");
// "chat" command handler
void chat(int, string);
// "addfriend" command handler
void addfriend(int, string);
// "creategroup" command handler
void creategroup(int, string);
// "addgroup" command handler
void addgroup(int, string);
// "groupchat" command handler
void groupchat(int, string);
// "loginout" command handler
void loginout(int, string);

// 系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令,格式help"},
    {"chat", "一对一聊天,格式chat:friendid:message"},
    {"addfriend", "添加好友,格式addfriend:friendid"},
    {"creategroup", "创建群组,格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组,格式addgroup:groupid"},
    {"groupchat", "群聊,格式groupchat:groupid:message"},
    {"loginout", "注销,格式loginout"}};

//注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}};

//主聊天页面
void mainMenu(int clientfd)
{
    help();

    char buff[1024] = {0};
    while (isMainMenuRunning) //注销之后，重新进入主菜单页面
    {
        cin.getline(buff, 1024);
        string commandBuff(buff);
        string command;
        int index = commandBuff.find(":");
        if (index == -1)
        {
            command = commandBuff;
        }
        else
        {
            command = commandBuff.substr(0, index);
        }

        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            cerr << "invalid input command." << endl;
            continue;
        }

        //调用命令处理方法：添加新功能不需要修改
        it->second(clientfd, commandBuff.substr(index + 1, commandBuff.size() - index));
    }
}

//帮助页面
void help(int, string)
{
    cout << "show command list." << endl;

    for (auto &p : commandMap)
    {
        cout << p.first << ": " << p.second << endl;
    }
    cout << "\n";
}

//添加好友 命令handler
void addfriend(int clientfd, string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string buff = js.dump();

    int len = send(clientfd, buff.c_str(), strlen(buff.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addfriend msg error -> " << buff << endl;
    }
}

//聊天 命令handler
void chat(int clientfd, string str)
{
    // friendid:message
    int index = str.find(":");
    if (-1 == index)
    {
        cerr << "chat command invalid!" << endl;
        return;
    }

    int friendid = atoi(str.substr(0, index).c_str());
    string message = str.substr(index + 1, str.size() - index);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buff = js.dump();

    int len = send(clientfd, buff.c_str(), strlen(buff.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send chat msg error -> " << buff << endl;
    }
}

//创建组 命令handler
void creategroup(int clientfd, string str)
{
    // groupidname:groupdesc
    int index = str.find(":");
    if (-1 == index)
    {
        cerr << "creategroup command invalid!" << endl;
        return;
    }

    string groupname = str.substr(0, index);
    string groupdesc = str.substr(index + 1, str.size() - index);

    json js;
    js["msgid"] = CREATE_CROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buff = js.dump();

    int len = send(clientfd, buff.c_str(), strlen(buff.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send creategroup msg error -> " << buff << endl;
    }
}

//加入群组 命令handler
void addgroup(int clientfd, string str)
{
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    string buff = js.dump();

    int len = send(clientfd, buff.c_str(), strlen(buff.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addgroup msg error -> " << buff << endl;
    }
}

//群聊服务 命令handler
void groupchat(int clientfd, string str)
{
    // groupid:message
    int index = str.find(":");
    if (-1 == index)
    {
        cerr << "groupchat command invalid!" << endl;
        return;
    }

    int groupid = atoi(str.substr(0, index).c_str());
    string message = str.substr(index + 1, str.size() - index);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buff = js.dump();

    int len = send(clientfd, buff.c_str(), strlen(buff.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send groupchat msg error -> " << buff << endl;
    }
}

//注销 命令handler
void loginout(int clientfd, string str)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    string buff = js.dump();

    int len = send(clientfd, buff.c_str(), strlen(buff.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send loginout msg error -> " << buff << endl;
    }
    else
    {
        isMainMenuRunning = false;
    }
}

//获取系统当前时间
string getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);

    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}