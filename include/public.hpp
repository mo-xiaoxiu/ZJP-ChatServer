#ifndef PUBLIC_H
#define PUBLIC_H

/*
server和client的公共文件
*/
enum EnMsgType
{
    LOGIN_MSG = 1, // 登录消息
    LOGIN_MSG_ACK, //登录响应
    REG_MSG, // 注册消息
    REG_MSG_ACK, //注册响应
    ONE_CHAT_MSG, //一对一聊天服务
    ADD_FRIEND_MSG, //添加号好友消息

    CREATE_CROUP_MSG, //创建群组消息
    ADD_GROUP_MSG, //加入群组消息
    GROUP_CHAT_MSG, //群聊天消息
    LOGINOUT_MSG //注销消息（客户）
};

#endif