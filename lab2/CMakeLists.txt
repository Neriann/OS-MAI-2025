cmake_minimum_required(VERSION 3.30)
project(lab2 C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_executable(lab2
        include/generate_test.h
        src/generate_test.c
        src/main.c)
