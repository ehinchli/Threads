cmake_minimum_required(VERSION 3.24)
set(CMAKE_C_STANDARD 23)
project(Assign3 C)


add_executable(Assign3 threads-grad.ehinchli.c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -g -O0")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -Ofast")