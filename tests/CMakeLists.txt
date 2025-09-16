cmake_minimum_required(VERSION 3.15)
project(XuanYuTests)

if(NOT BUILD_TESTS)
    return()
endif()

enable_testing()

# Placeholder test target
add_executable(unit_tests ${CMAKE_CURRENT_SOURCE_DIR}/placeholder.cpp)
add_test(NAME placeholder_test COMMAND unit_tests)
