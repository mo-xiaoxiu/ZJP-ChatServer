#ifndef _USERMODEL_H_
#define _USERMODEL_H_

#include "user.hpp"

class UserModel
{
public:
    bool insert(User& user);

    User query(int id);

    bool updateState(User& user);

    //重置状态信息
    void resetState();
//private:    
};

#endif
