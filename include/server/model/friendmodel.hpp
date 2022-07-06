#ifndef _FRIENDMODEL_H_
#define _FRIENDMODEL_H_


#include <vector>
#include "user.hpp"

//处理好友请求（添加、查询）
class FriendModel
{
public:
    //添加好友
    void insert(int userid, int friendid);

    //返回好友列表
    vector<User> query(int userid);
};


#endif