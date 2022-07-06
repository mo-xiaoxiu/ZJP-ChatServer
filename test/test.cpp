#include "db.hpp"
#include <muduo/base/Logging.h>
#include <memory.h>
#include <thread>
#include <mutex>
using namespace std;

static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string dbname = "chat";

void testMysqlConnection()
{
    // connect
    MySQL mysql;
    if (mysql.connect())
    {
        LOG_INFO << "connected successfully.";
    }
    else
    {
        LOG_INFO << "connected failed.";
    }
}

void testMysqlUpdate()
{
    // update
    MySQL mysql;
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')",
            "zjp-test", "123456", "offline");

    if (mysql.update(sql))
    {
        LOG_INFO << "updated successfully.";
    }
    else
    {
        LOG_INFO << "updated failed.";
    }
}

//测试数据库查询
void testMysqlQuery(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where id = %d", id);

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.query(sql))
        {
            LOG_INFO << "updated successfully.";
        }
        else
        {
            LOG_INFO << "updated failed.";
        }
    }
}

int main()
{

    //testMysqlConnection();

    // testMysqlUpdate();

    testMysqlQuery(22);

    return 0;
}