#include "offlinemsgmodel.hpp"
#include <iostream>
#include "db.hpp"

//存储离线消息
void OfflineMsgModel::insertofflinemsg(int id, string msg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage values('%d', '%s')", id, msg.c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

//查询离线消息
vector<string> OfflineMsgModel::queryofflinemsg(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where userid = %d", id);

    MySQL mysql;
    vector<string> vec;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr) {
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}

//删除数据库中的离线消息
void OfflineMsgModel::removeofflinemsg(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid=%d", id);
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}