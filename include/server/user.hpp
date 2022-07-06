#ifndef _USER_H_
#define _USER_H_

#include <string>
using namespace std;

class User
{
public:
    User(int id = -1, string name = "", string pwd = "", string state = "offline")
    : m_id(id), m_name(name), m_pwd(pwd), m_state(state) {}

    void setId(int id) { m_id = id; }
    void setName(string name) { m_name = name; }
    void setPwd(string pwd) { m_pwd = pwd; }
    void setState(string state) { m_state = state; }

    int getId() { return m_id; }
    string getName() { return m_name; }
    string getPwd() { return m_pwd; }
    string getState() { return m_state; }

private : 
    int m_id;
    string m_name;
    string m_pwd;
    string m_state;
};

#endif
