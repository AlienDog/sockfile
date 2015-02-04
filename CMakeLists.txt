cmake_minimum_required(VERSION 2.8.4)
project(sockfile)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES source/main.cpp)
add_executable(netinstall ${SOURCE_FILES})