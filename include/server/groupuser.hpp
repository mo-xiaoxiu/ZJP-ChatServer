#ifndef _GROUPUSER_H_
#define _GROUPUSER_H_

#include "user.hpp"

//群组成员：比一般的user多了一个role（在群组中承担的角色）
class GroupUser: public User
{
public:
    void setRole(string){this->role = role;}
    string getRole(){return this->role;}

private:
    string role;    
};

#endif