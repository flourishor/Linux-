cmake_minimum_required(VERSION 3.16)  # Ubuntu 20.04 默认 CMake 3.16.3
project(my_qq_1_3_server)

set(CMAKE_CXX_STANDARD 20)

# 添加可执行文件
add_executable(my_qq_1_3_server 
        Server.cpp
        ThreadPool.h
        ThreadPool.cpp
        ConnectionPool.h
        ConnectionPool.cpp
)
# 找到 MySQL 头文件
include_directories(/usr/include/mysql)

# 连接 MySQL 客户端库
target_link_libraries(my_qq_1_3_server mysqlclient pthread)

