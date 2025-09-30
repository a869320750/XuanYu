cmake_minimum_required(VERSION 3.10)
project(UAVChipAuth)

# 设置C标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 是否静态化 libstdc++/libgcc，用于减轻目标系统的 glibc 依赖（默认开启）
option(UAV_STATIC_LIBS "Use static libstdc++/libgcc" ON)

# 平台选择：x86 或 arm
if(NOT DEFINED TARGET_PLATFORM)
    set(TARGET_PLATFORM "x86" CACHE STRING "Target platform: x86 or arm")
endif()

# 链接方式选择：static 或 dynamic
if(NOT DEFINED LINK_TYPE)
    set(LINK_TYPE "static" CACHE STRING "Link type: static or dynamic")
endif()

# 根据平台设置交叉编译工具链
if(TARGET_PLATFORM STREQUAL "arm")
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR arm)
    
    # 交叉编译工具链
    set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
    set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
    
    # 编译参数
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -g -march=armv7-a -mfpu=neon -mfloat-abi=hard")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g -march=armv7-a -mfpu=neon -mfloat-abi=hard")
    
    message(STATUS "配置ARM交叉编译环境")
else()
    # x86编译参数
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g")
    message(STATUS "配置x86编译环境")
endif()

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/doc/DMT_SDK/usr)

# 根据平台设置库文件路径
if(TARGET_PLATFORM STREQUAL "arm")
    link_directories(${CMAKE_SOURCE_DIR}/lib/arm32v7)
    set(LIB_PATH "${CMAKE_SOURCE_DIR}/lib/arm32v7")
else()
    link_directories(${CMAKE_SOURCE_DIR}/lib/x86)
    set(LIB_PATH "${CMAKE_SOURCE_DIR}/lib/x86")
endif()

# 源文件
set(SOURCES
    src/main.cpp
    src/trans.c
    src/AuthClient.cpp
    src/HardwareAdapter.cpp
)

# include for our headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# add the new AuthClient.cpp
list(APPEND SOURCES src/AuthClient.cpp)

# 创建可执行文件
add_executable(uavchip-auth ${SOURCES})

# 对C++构建的目标，尝试静态化 libstdc++ / libgcc 来减少对更高版本 glibc 的依赖。
# 这通常能解决交叉 g++ 帶来的 GLIBC_2.38 要求问题（快速修复）。
if(UAV_STATIC_LIBS)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER MATCHES ".*g\\+\\+.*")
        set_target_properties(uavchip-auth PROPERTIES
                LINK_FLAGS "-static-libgcc -static-libstdc++"
        )
    endif()
endif()

# 检查并链接库文件 - 根据LINK_TYPE选择链接方式
if(LINK_TYPE STREQUAL "static" AND EXISTS ${LIB_PATH}/libdmtcryptc.a)
    # 优先静态链接
    target_link_libraries(uavchip-auth ${LIB_PATH}/libdmtcryptc.a)
    message(STATUS "静态链接: ${LIB_PATH}/libdmtcryptc.a")
    
    # 设置静态链接选项
    set_target_properties(uavchip-auth PROPERTIES LINK_FLAGS "-static-libgcc -static-libstdc++")
    
elseif(EXISTS ${LIB_PATH}/libdmtcryptc.so)
    # 动态链接
    target_link_libraries(uavchip-auth dmtcryptc)
    message(STATUS "动态链接: ${LIB_PATH}/libdmtcryptc.so")
    message(WARNING "使用动态库，运行时需要确保库文件在系统路径中")
    
elseif(EXISTS ${LIB_PATH}/libdmtcryptc.a)
    # 回退到静态链接
    target_link_libraries(uavchip-auth ${LIB_PATH}/libdmtcryptc.a)
    message(STATUS "回退到静态链接: ${LIB_PATH}/libdmtcryptc.a")
else()
    message(WARNING "未找到libdmtcryptc库文件，部分功能可能无法使用")
endif()

# 设置输出目录（根据平台区分）
set_target_properties(uavchip-auth PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${TARGET_PLATFORM}
)

# 安装规则
install(TARGETS uavchip-auth
    RUNTIME DESTINATION bin/${TARGET_PLATFORM}
)

# 打印构建信息
message(STATUS "Target Platform: ${TARGET_PLATFORM}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "Library Path: ${LIB_PATH}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C Flags: ${CMAKE_C_FLAGS}")
