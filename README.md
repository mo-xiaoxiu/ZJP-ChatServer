# ZJP-ChatServer

<br>

## 基本描述

C++基于控制台的集群聊天服务器：以配置nginx负载均衡模块分发客户端连接、基于发布-订阅的redis作为服务器集群消息队列，以mysql数据库存储用户数据

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
│   ├── public.hpp
│   └── server
│       ├── chatserver.hpp
│       ├── chatservice.hpp
│       ├── db
│       │   └── db.hpp
│       ├── group.hpp
│       ├── groupuser.hpp
│       ├── model
│       │   ├── friendmodel.hpp
│       │   ├── groupmodel.hpp
│       │   ├── offlinemsgmodel.hpp
│       │   └── usermodel.hpp
│       ├── redis
│       │   └── redis.hpp
│       └── user.hpp
├── src
│   ├── client
│   │   ├── CMakeLists.txt
│   │   └── main.cpp
│   ├── CMakeLists.txt
│   └── server
│       ├── chatserver.cpp
│       ├── chatservice.cpp
│       ├── CMakeLists.txt
│       ├── db
│       │   └── db.cpp
│       ├── main.cpp
│       ├── model
│       │   ├── friendmodel.cpp
│       │   ├── groupmodel.cpp
│       │   ├── offlinemsgmodel.cpp
│       │   └── usermodel.cpp
│       └── redis
│           └── redis.cpp
├── test
│   ├── CMakeLists.txt
│   ├── test_build
│   │   ├── CMakeCache.txt
│   │   ├── CMakeFiles
│   │   │   ├── 3.24.0-rc2
│   │   │   │   ├── CMakeCCompiler.cmake
│   │   │   │   ├── CMakeCXXCompiler.cmake
│   │   │   │   ├── CMakeDetermineCompilerABI_C.bin
│   │   │   │   ├── CMakeDetermineCompilerABI_CXX.bin
│   │   │   │   ├── CMakeSystem.cmake
│   │   │   │   ├── CompilerIdC
│   │   │   │   │   ├── a.out
│   │   │   │   │   ├── CMakeCCompilerId.c
│   │   │   │   │   └── tmp
│   │   │   │   └── CompilerIdCXX
│   │   │   │       ├── a.out
│   │   │   │       ├── CMakeCXXCompilerId.cpp
│   │   │   │       └── tmp
│   │   │   ├── cmake.check_cache
│   │   │   ├── CMakeDirectoryInformation.cmake
│   │   │   ├── CMakeOutput.log
│   │   │   ├── CMakeTmp
│   │   │   ├── Makefile2
│   │   │   ├── Makefile.cmake
│   │   │   ├── pkgRedirects
│   │   │   ├── progress.marks
│   │   │   ├── TargetDirectories.txt
│   │   │   └── test.dir
│   │   │       ├── build.make
│   │   │       ├── cmake_clean.cmake
│   │   │       ├── compiler_depend.internal
│   │   │       ├── compiler_depend.make
│   │   │       ├── compiler_depend.ts
│   │   │       ├── DependInfo.cmake
│   │   │       ├── depend.make
│   │   │       ├── flags.make
│   │   │       ├── link.txt
│   │   │       ├── progress.make
│   │   │       ├── root
│   │   │       │   └── test
│   │   │       │       └── chat
│   │   │       │           └── src
│   │   │       │               └── server
│   │   │       │                   └── db
│   │   │       │                       ├── db.cpp.o
│   │   │       │                       └── db.cpp.o.d
│   │   │       ├── test.cpp.o
│   │   │       └── test.cpp.o.d
│   │   ├── cmake_install.cmake
│   │   ├── Makefile
│   │   └── test
│   └── test.cpp
└── thirdparty
    └── json.hpp

31 directories, 64 files
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

### 配置环境MySQL

---

`...`
