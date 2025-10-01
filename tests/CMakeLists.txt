cmake_minimum_required(VERSION 3.15)

# 查找GoogleTest
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# 设置测试输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR})

# 测试源文件
set(TEST_SOURCES
    test_main.cpp
    transport/test_transport_socket.cpp
    crypto/test_crypto_software.cpp
    crypto/test_software_hardware_consistency.cpp
    communication/test_secure_client.cpp
    communication/test_secure_server.cpp
    mocks/MockTransportAdapter.cpp
)

# 创建测试可执行文件
add_executable(xuanyu_tests ${TEST_SOURCES})

# 链接库
target_link_libraries(xuanyu_tests
    PRIVATE
        xuanyu_static
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        Threads::Threads
)

# 设置测试属性
set_target_properties(xuanyu_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# 添加编译选项
target_compile_options(xuanyu_tests PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# 覆盖率支持
if(ENABLE_COVERAGE)
    target_compile_options(xuanyu_tests PRIVATE --coverage)
    target_link_libraries(xuanyu_tests PRIVATE --coverage)
endif()

# 注册测试
include(GoogleTest)
gtest_discover_tests(xuanyu_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

# 创建自定义测试目标
add_custom_target(run_tests
    COMMAND xuanyu_tests
    DEPENDS xuanyu_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    COMMENT "Running unit tests"
)

# 覆盖率报告
if(ENABLE_COVERAGE)
    find_program(GCOV_PATH gcov)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(GCOV_PATH AND LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage
            COMMAND ${LCOV_PATH} --directory . --zerocounters
            COMMAND ${CMAKE_COMMAND} --build . --target run_tests
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info '*/tests/*' --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info '*/build/*' --output-file coverage.info
            COMMAND ${GENHTML_PATH} -o coverage_html coverage.info
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating code coverage report"
        )
    endif()
endif()