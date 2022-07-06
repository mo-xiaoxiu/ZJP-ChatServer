#ifndef _OFFLINGMSGMODEL_H_
#define _OFFLINEMSGMODEL_H_

#include <vector>
#include <string>
using namespace std;


//处理离线消息
class OfflineMsgModel
{
public:
    //存储离线消息
    void insertofflinemsg(int id, string msg);

    //查询离线消息
    vector<string> queryofflinemsg(int id);

    //删除数据库中的离线消息
    void removeofflinemsg(int id);
};

#endif