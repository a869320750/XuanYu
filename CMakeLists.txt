cmake_minimum_required(VERSION 3.15)

project(XuanYu VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# 项目选项
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
option(USE_HARDWARE_CRYPTO "Use hardware crypto implementation" OFF)

# 输出目录设置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 查找依赖包
find_package(PkgConfig REQUIRED)

# 查找GmSSL
set(GMSSL_PKG_OK FALSE)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(GMSSL QUIET gmssl)
    if(GMSSL_FOUND)
        set(GMSSL_PKG_OK TRUE)
    endif()
endif()

if(NOT GMSSL_PKG_OK)
    message(STATUS "pkg-config did not find gmssl, trying fallback search for headers and libraries...")
    # 常见头文件路径（增加更多候选名和路径）
    find_path(GMSSL_INCLUDE_DIRS
        NAMES gmssl/ssl.h gmssl.h openssl/ssl.h gmssl/sm2.h
        PATHS /usr/local/include /usr/include /usr/local/opt/openssl/include /usr/local/include/gmssl /usr/local/include/guanzhi
        PATH_SUFFIXES include include/gmssl
    )

    # 常见库路径（增加更多候选名和路径）
    find_library(GMSSL_LIBRARIES
        NAMES gmssl libgmssl gmssl_static
        PATHS /usr/local/lib /usr/lib /usr/local/opt/openssl/lib /usr/local/lib64 /usr/lib64
        PATH_SUFFIXES lib lib64
    )

    if(GMSSL_INCLUDE_DIRS AND GMSSL_LIBRARIES)
        message(STATUS "Found GmSSL headers in ${GMSSL_INCLUDE_DIRS} and libs: ${GMSSL_LIBRARIES}")
        set(GMSSL_PKG_OK TRUE)
        # 定义HAVE_GMSSL宏
        add_definitions(-DHAVE_GMSSL=1)
    else()
        message(FATAL_ERROR "GmSSL not found. Please install GmSSL (and ensure pkg-config can find it) or set GMSSL_INCLUDE_DIRS/GMSSL_LIBRARIES.\nHint: check /usr/local/include, /usr/local/lib, and run 'pkg-config --list-all | grep gmssl' inside the container. You can also set -DGMSSL_INCLUDE_DIRS=/path -DGMSSL_LIBRARIES=/path to help CMake find it.")
    endif()
else()
    message(STATUS "GmSSL found via pkg-config: includes=${GMSSL_INCLUDE_DIRS}, libs=${GMSSL_LIBRARIES}")
    # 定义HAVE_GMSSL宏
    add_definitions(-DHAVE_GMSSL=1)
endif()

# 查找JSON库
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    # 如果找不到，使用FetchContent下载
    include(FetchContent)
    FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
    )
    FetchContent_MakeAvailable(json)
endif()

# 查找线程库
find_package(Threads REQUIRED)

# 源文件
set(XUANYU_SOURCES
    src/crypto/CryptoSoftware.cpp
    src/communication/SecureBase.cpp
    src/communication/SecureClient.cpp
    src/communication/SecureServer.cpp
)

# 头文件
set(XUANYU_HEADERS

    include/crypto/ICryptoProvider.h
    include/crypto/CryptoSoftware.h
    include/communication/SecureBase.h
    include/communication/SecureClient.h
    include/communication/SecureServer.h
)

# 创建库
add_library(xuanyu SHARED ${XUANYU_SOURCES})
add_library(xuanyu_static STATIC ${XUANYU_SOURCES})

# 设置库属性
set_target_properties(xuanyu PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${XUANYU_HEADERS}"
)

set_target_properties(xuanyu_static PROPERTIES
    OUTPUT_NAME xuanyu
    PUBLIC_HEADER "${XUANYU_HEADERS}"
)

# 链接库
target_link_libraries(xuanyu 
    PRIVATE 
        ${GMSSL_LIBRARIES}
        Threads::Threads
    PUBLIC
        nlohmann_json::nlohmann_json
)

target_link_libraries(xuanyu_static 
    PRIVATE 
        ${GMSSL_LIBRARIES}
        Threads::Threads
    PUBLIC
        nlohmann_json::nlohmann_json
)

# 包含目录
target_include_directories(xuanyu
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${GMSSL_INCLUDE_DIRS}
)

target_include_directories(xuanyu_static
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${GMSSL_INCLUDE_DIRS}
)

# 编译定义
target_compile_definitions(xuanyu PRIVATE XUANYU_EXPORTS)
if(USE_HARDWARE_CRYPTO)
    target_compile_definitions(xuanyu PUBLIC USE_HARDWARE_CRYPTO)
    target_compile_definitions(xuanyu_static PUBLIC USE_HARDWARE_CRYPTO)
endif()

# 覆盖率支持
if(ENABLE_COVERAGE)
    target_compile_options(xuanyu PRIVATE --coverage)
    target_link_libraries(xuanyu PRIVATE --coverage)
    target_compile_options(xuanyu_static PRIVATE --coverage)
    target_link_libraries(xuanyu_static PRIVATE --coverage)
endif()

# 测试
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# 示例
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# 基准测试
if(BUILD_BENCHMARKS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks")
    add_subdirectory(benchmarks)
endif()

# 安装配置
include(GNUInstallDirs)

install(TARGETS xuanyu xuanyu_static
    EXPORT XuanYuTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xuanyu
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xuanyu
    FILES_MATCHING PATTERN "*.h"
)

# 导出配置
install(EXPORT XuanYuTargets
    FILE XuanYuTargets.cmake
    NAMESPACE XuanYu::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/XuanYu
)

# 创建配置文件
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/XuanYuConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/XuanYuConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/XuanYu
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/XuanYuConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/XuanYuConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/XuanYuConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/XuanYu
)

# 创建pkg-config文件
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/xuanyu.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/xuanyu.pc"
    @ONLY
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/xuanyu.pc"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# CPack配置
set(CPACK_PACKAGE_NAME "XuanYu")
set(CPACK_PACKAGE_VENDOR "XuanYu Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "High Security Communication Framework")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "XuanYu")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

include(CPack)

# 打印配置信息
message(STATUS "")
message(STATUS "========== XuanYu Build Configuration ==========")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "Build examples: ${BUILD_EXAMPLES}")
message(STATUS "Build benchmarks: ${BUILD_BENCHMARKS}")
message(STATUS "Enable coverage: ${ENABLE_COVERAGE}")
message(STATUS "Use hardware crypto: ${USE_HARDWARE_CRYPTO}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "============================================")
message(STATUS "")
