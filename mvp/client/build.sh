#!/bin/bash

# UAVChip-Auth 自动编译脚本
# 使用方法: ./build.sh [clean|debug|release] [x86|arm]

PROJECT_NAME="uavchip-auth"
BUILD_DIR="build"

# 默认参数
BUILD_TYPE="Debug"
TARGET_PLATFORM="x86"

# 解析命令行参数
for arg in "$@"; do
    case "$arg" in
        "clean")
            echo "清理构建文件..."
            rm -rf $BUILD_DIR
            echo "清理完成"
            exit 0
            ;;
        "debug")
            BUILD_TYPE="Debug"
            ;;
        "release")
            BUILD_TYPE="Release"
            ;;
        "x86")
            TARGET_PLATFORM="x86"
            ;;
        "arm")
            TARGET_PLATFORM="arm"
            ;;
        *)
            if [[ "$arg" != "" ]]; then
                echo "用法: $0 [clean|debug|release] [x86|arm]"
                echo "  clean   - 清理构建文件"
                echo "  debug   - Debug模式编译（默认）"
                echo "  release - Release模式编译"
                echo "  x86     - x86平台编译（默认）"
                echo "  arm     - ARM平台交叉编译"
                echo ""
                echo "示例:"
                echo "  $0 debug x86    # x86 Debug编译"
                echo "  $0 release arm  # ARM Release编译"
                exit 1
            fi
            ;;
    esac
done

BIN_DIR="$BUILD_DIR/bin/$TARGET_PLATFORM"

# 检查必要工具
command -v cmake >/dev/null 2>&1 || { echo "错误: 未找到cmake，请先安装"; exit 1; }
command -v make >/dev/null 2>&1 || { echo "错误: 未找到make，请先安装"; exit 1; }

# 如果是ARM编译，检查交叉编译工具链
if [ "$TARGET_PLATFORM" = "arm" ]; then
    command -v arm-linux-gnueabihf-gcc >/dev/null 2>&1 || { 
        echo "错误: 未找到ARM交叉编译工具链 (arm-linux-gnueabihf-gcc)"
        echo "请安装ARM交叉编译工具链，例如:"
        echo "  sudo apt-get install gcc-arm-linux-gnueabihf"
        exit 1
    }
fi

echo "========================================="
echo "  UAVChip-Auth 自动编译脚本"
echo "========================================="
echo "项目名称: $PROJECT_NAME"
echo "目标平台: $TARGET_PLATFORM"
echo "构建类型: $BUILD_TYPE"
echo "构建目录: $BUILD_DIR"
echo "输出目录: $BIN_DIR"
echo "========================================="

# 创建构建目录
if [ ! -d "$BUILD_DIR" ]; then
    echo "创建构建目录: $BUILD_DIR"
    mkdir -p $BUILD_DIR
fi

# 进入构建目录
cd $BUILD_DIR

# 运行cmake配置
echo "正在配置项目..."
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DTARGET_PLATFORM=$TARGET_PLATFORM .. || {
    echo "错误: cmake配置失败"
    exit 1
}

# 编译项目
echo "正在编译项目..."
make -j$(nproc) || {
    echo "错误: 编译失败"
    exit 1
}

# 检查编译结果
if [ -f "bin/$TARGET_PLATFORM/uavchip-auth" ]; then
    echo "========================================="
    echo "编译成功！"
    echo "可执行文件: $BIN_DIR/uavchip-auth"
    echo "========================================="
    
    # 显示文件信息
    echo "文件信息:"
    ls -la bin/$TARGET_PLATFORM/uavchip-auth

else
    echo "错误: 未找到编译后的可执行文件"
    echo "预期路径: bin/$TARGET_PLATFORM/uavchip-auth"
    echo "当前目录内容:"
    ls -la bin/ 2>/dev/null || echo "bin目录不存在"
    exit 1
fi
