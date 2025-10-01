# Examples and benchmark applications

# 加密功能演示程序
add_executable(crypto_demo crypto_demo.cpp)

target_link_libraries(crypto_demo
    PRIVATE
        xuanyu_static
        ${GMSSL_LIBRARIES}
        Threads::Threads
)

target_include_directories(crypto_demo
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${GMSSL_INCLUDE_DIRS}
)

# 加密基准测试程序
add_executable(crypto_benchmark crypto_benchmark.cpp)

target_link_libraries(crypto_benchmark
    PRIVATE
        xuanyu_static
        ${GMSSL_LIBRARIES}
        Threads::Threads
)

target_include_directories(crypto_benchmark
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${GMSSL_INCLUDE_DIRS}
)

# 设置输出目录
set_target_properties(crypto_demo crypto_benchmark
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 安装示例程序
install(TARGETS crypto_demo crypto_benchmark
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
