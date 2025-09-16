#!/bin/bash

# XuanYu Docker容器入口脚本

set -e

echo "=========================================="
echo "  XuanYu 高安全通信框架开发环境"
echo "=========================================="
echo "容器启动时间: $(date)"
echo "用户: $(whoami)"
echo "工作目录: $(pwd)"
echo "=========================================="

# 显示版本信息
echo "环境信息:"
echo "  - 操作系统: $(lsb_release -d | cut -f2)"
echo "  - GCC版本: $(gcc --version | head -n1)"
echo "  - CMake版本: $(cmake --version | head -n1)"
echo "  - GmSSL版本: $(gmssl version 2>/dev/null || echo 'GmSSL not in PATH')"

# 导出 pkg-config 路径，确保在 /usr/local 安装的 gmssl 能被找到
export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:/usr/local/share/pkgconfig:${PKG_CONFIG_PATH}"


# 检查源码是否挂载
if [ -f "/workspace/CMakeLists.txt" ]; then
    echo "  - 源码目录: ✓ 已挂载"
else
    echo "  - 源码目录: ✗ 未挂载"
    echo "    提示: 使用 -v $(pwd):/workspace 挂载源码目录"
fi

echo "=========================================="

# 如果传入了参数，执行对应命令
if [ $# -gt 0 ]; then
    case "$1" in
        "build")
            echo "执行构建..."
            cd /workspace
            mkdir -p build
            cd build
            cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
            make -j$(nproc)
            ;;
        "test")
            echo "执行测试..."
            cd /workspace/build
            if [ ! -f "Makefile" ]; then
                echo "未找到构建文件，先执行构建..."
                cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
                make -j$(nproc)
            fi
            make test
            ;;
        "coverage")
            echo "生成覆盖率报告..."
            cd /workspace
            mkdir -p build
            cd build
            cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON -DBUILD_TESTS=ON
            make -j$(nproc)
            make test
            make coverage
            ;;
        "benchmark")
            echo "执行性能测试..."
            cd /workspace/build
            if [ ! -f "Makefile" ]; then
                cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON
                make -j$(nproc)
            fi
            make benchmark
            ;;
        "clean")
            echo "清理构建文件..."
            rm -rf /workspace/build/*
            ;;
        "shell")
            echo "启动交互shell..."
            exec /bin/bash
            ;;
        *)
            echo "执行命令: $@"
            exec "$@"
            ;;
    esac
else
    # 默认启动交互shell
    echo "启动开发环境shell..."
    echo "可用命令:"
    echo "  docker exec <container> /usr/local/bin/entrypoint.sh build     # 构建项目"
    echo "  docker exec <container> /usr/local/bin/entrypoint.sh test      # 运行测试"
    echo "  docker exec <container> /usr/local/bin/entrypoint.sh coverage  # 生成覆盖率"
    echo "  docker exec <container> /usr/local/bin/entrypoint.sh benchmark # 性能测试"
    echo "  docker exec <container> /usr/local/bin/entrypoint.sh clean     # 清理构建"
    echo ""
    exec /bin/bash
fi

# 尝试修复挂载目录的权限问题（只有在容器以root用户启动时有效）
if [ "$(id -u)" = "0" ]; then
    if [ -d "/workspace" ]; then
        mkdir -p /workspace/build || true
        chown -R xuanyu:xuanyu /workspace || true
    fi
fi
