# ZJP-ChatServer

<br>

## 基本描述

C++基于控制台的集群聊天服务器：以配置nginx负载均衡模块分发客户端连接、基于发布-订阅的redis作为服务器集群消息队列，以mysql数据库存储用户数据

### 环境

* Ubuntu 18
* gcc 7.5.0
* cmake 3.24.0

### 技术

* 使用Json序列化和反序列化发送的数据
* muduo网络库的开发基本使用
* MySQL数据库编程
* ngix tcp模块配置编译实现负载均衡
* 发布-订阅消息的redis实现不同服务器上客户连接的消息发送

### 文件目录结构

```
root@iZwz959g1zzxerwo9dtx98Z:~/test/chat# tree .
.
├── bin
├── build
├── build.sh
├── CMakeLists.txt
├── include
│?? ├── public.hpp
│?? └── server
│??     ├── chatserver.hpp
│??     ├── chatservice.hpp
│??     ├── db
│??     │?? └── db.hpp
│??     ├── group.hpp
│??     ├── groupuser.hpp
│??     ├── model
│??     │?? ├── friendmodel.hpp
│??     │?? ├── groupmodel.hpp
│??     │?? ├── offlinemsgmodel.hpp
│??     │?? └── usermodel.hpp
│??     ├── redis
│??     │?? └── redis.hpp
│??     └── user.hpp
├── src
│?? ├── client
│?? │?? ├── CMakeLists.txt
│?? │?? └── main.cpp
│?? ├── CMakeLists.txt
│?? └── server
│??     ├── chatserver.cpp
│??     ├── chatservice.cpp
│??     ├── CMakeLists.txt
│??     ├── db
│??     │?? └── db.cpp
│??     ├── main.cpp
│??     ├── model
│??     │?? ├── friendmodel.cpp
│??     │?? ├── groupmodel.cpp
│??     │?? ├── offlinemsgmodel.cpp
│??     │?? └── usermodel.cpp
│??     └── redis
│??         └── redis.cpp
├── test
│?? ├── CMakeLists.txt
│?? ├── test_build
│?? └── test.cpp
└── thirdparty
    └── json.hpp

16 directories, 30 files

```

<br>

## 编译和配置

### 编译

1. 可以使用一键编译：

   ```shell
   sh build.sh
   ```

2. 可以进入到`build`文件中：

   ```shell
   cmake ..
   make
   ```

### MySQL

1. 表设计

   `user`

   ![](https://myblog-1308923350.cos.ap-guangzhou.myqcloud.com/img/user.png)

   `friend`

   ![](https://myblog-1308923350.cos.ap-guangzhou.myqcloud.com/img/friend.png)

   `groupuser`

   ![](https://myblog-1308923350.cos.ap-guangzhou.myqcloud.com/img/groupuser.png)

   `allgroup`

   ![](https://myblog-1308923350.cos.ap-guangzhou.myqcloud.com/img/allgroup.png)

   `offlinemessage`

   ![](https://myblog-1308923350.cos.ap-guangzhou.myqcloud.com/img/offlinemessage.png)

### Nginx

```
./configuer --with-stream
make && make install
```

```
cd /usr/local/nginx/
```

修改配置文件：

```
cd conf/
vim nginx.conf
```

加入模块：

```
# nginx tcp loadbalance config
stream {
    upstream MyServer {
        server 127.0.0.1:6000 weight=1 max_fails=3 fail_timeout=30s;
        server 127.0.0.1:6002 weight=1 max_fails=3 fail_timeout=30s;
    }

    server {
        proxy_connect_timeout 1s;
        listen 8000;
        proxy_pass MyServer;
        tcp_nodelay on;
    }
}
```

启动nginx：

```
pwd
/usr/local/nginx/conf
cd ../sbin/
./nginx
```

重新启动和停止服务：

```
nginx -s reload 重新加载配置文件启动
nginx -s stop 停止nginx服务
```

检查nginx状态：

```
root@iZwz959g1zzxerwo9dtx98Z:/usr/local/nginx/conf# netstat -anpt | grep nginx
tcp        0      0 0.0.0.0:8000            0.0.0.0:*               LISTEN      15454/nginx: master 
tcp        0      0 0.0.0.0:80              0.0.0.0:*               LISTEN      15454/nginx: master 
```

### Redis

```
root@iZwz959g1zzxerwo9dtx98Z:/usr/local/nginx/conf# netstat -anpt | grep redis
tcp        0      0 127.0.0.1:6379          0.0.0.0:*               LISTEN      598/redis-server 12 
tcp6       0      0 ::1:6379                :::*                    LISTEN      598/redis-server 12 
```





## 部分图解

### Chatservice

![](https://myblog-1308923350.cos.ap-guangzhou.myqcloud.com/img/chatservice.png)

### model

![](https://myblog-1308923350.cos.ap-guangzhou.myqcloud.com/img/model.png)

### Nginx

![](https://myblog-1308923350.cos.ap-guangzhou.myqcloud.com/img/Nginx.png)

### Redis

![](https://myblog-1308923350.cos.ap-guangzhou.myqcloud.com/img/Nginx_redis.png)

### client

![](https://myblog-1308923350.cos.ap-guangzhou.myqcloud.com/img/client_fin.png)

