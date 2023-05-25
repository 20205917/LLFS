cmake_minimum_required(VERSION 3.21)
project(LLFS)

set(CMAKE_CXX_STANDARD 14)

add_executable(LLFS main.cpp config.h fileOperation.cpp userManagement.cpp memoryManagement.cpp storageManagement.cpp)
