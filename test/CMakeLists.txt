cmake_minimum_required(VERSION 3.10)
project(test)

set(CMAKE_CXX_FLAGS "-std=c++11 -g")

set(SRC_FILE test.cpp ../src/server/db/db.cpp)
include_directories(../include/server/db)


add_executable(test ${SRC_FILE})
target_link_libraries(test mysqlclient muduo_base muduo_net pthread)