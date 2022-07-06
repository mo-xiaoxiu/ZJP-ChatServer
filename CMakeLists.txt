cmake_minimum_required(VERSION 3.10)
project(chat)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 设置头文件搜索路径
include_directories(include)
include_directories(include/server)
include_directories(include/server/db)
include_directories(include/server/model)
include_directories(include/server/redis)
include_directories(thirdparty)

# 设置可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 添加子目录
add_subdirectory(src)
