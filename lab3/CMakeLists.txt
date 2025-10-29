cmake_minimum_required(VERSION 3.30)
project(OS_LAB3 C)

set(CMAKE_C_STANDARD 11)

add_executable(client src/client.c)

add_executable(server src/server.c)

target_include_directories(client PUBLIC include)

target_include_directories(server PUBLIC include)
