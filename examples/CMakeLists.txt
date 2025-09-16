cmake_minimum_required(VERSION 3.15)
project(XuanYuExamples)

if(NOT BUILD_EXAMPLES)
    return()
endif()

add_executable(echo_server echo_server.cpp)
add_executable(echo_client echo_client.cpp)
