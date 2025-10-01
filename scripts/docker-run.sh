#!/bin/bash

# XuanYu Docker 运行脚本
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

IMAGE_NAME="xuanyu-dev"
CONTAINER_NAME="xuanyu-dev-container"

# 解析命令行参数
BUILD_IMAGE="false"
RUN_TESTS="false"
INTERACTIVE="false"
COMMAND=""
ARM_BUILD="false"

while [[ $# -gt 0 ]]; do
    case $1 in
        --build)
            BUILD_IMAGE="true"
            shift
            ;;
        --test)
            RUN_TESTS="true"
            shift
            ;;
        --test-arm)
            RUN_TESTS="true"
            ARM_BUILD="true"
            shift
            ;;
        -i|--interactive)
            INTERACTIVE="true"
            shift
            ;;
        --cmd)
            COMMAND="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --build       Build Docker image"
            echo "  --test        Run tests in container (native x86_64)"
            echo "  --test-arm    Run ARM32v7 cross-compilation test"
            echo "  -i, --interactive  Run interactive shell"
            echo "  --cmd COMMAND Run specific command"
            echo "  -h, --help    Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# 构建Docker镜像
if [[ "$BUILD_IMAGE" == "true" ]]; then
    echo "Building Docker image: $IMAGE_NAME"
    cd "$PROJECT_ROOT"
    docker build -t "$IMAGE_NAME" -f docker/Dockerfile .
    echo "Docker image built successfully"
fi

# 移除现有容器（如果存在）
if docker ps -a --format 'table {{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo "Removing existing container: $CONTAINER_NAME"
    docker rm -f "$CONTAINER_NAME"
fi

# 设置运行参数
DOCKER_ARGS=()
DOCKER_ARGS+=("--name" "$CONTAINER_NAME")
DOCKER_ARGS+=("--rm")
DOCKER_ARGS+=("-v" "$PROJECT_ROOT:/workspace")
DOCKER_ARGS+=("-w" "/workspace")

# 如果是交互式或没有指定命令，添加交互参数
if [[ "$INTERACTIVE" == "true" ]] || [[ -z "$COMMAND" && "$RUN_TESTS" == "false" ]]; then
    DOCKER_ARGS+=("-it")
fi

# 运行容器
if [[ "$RUN_TESTS" == "true" ]]; then
    if [[ "$ARM_BUILD" == "true" ]]; then
        echo "Running ARM32v7 cross-compilation test in Docker container..."
        docker run "${DOCKER_ARGS[@]}" "$IMAGE_NAME" bash -c "
            chmod +x scripts/build-cross.sh &&
            ./scripts/build-cross.sh --clean --arm32v7 &&
            echo '=== ARM32v7 cross-compilation completed ===' &&
            echo 'ARM binary info:' &&
            file build-arm/tests/xuanyu_tests || echo 'Test executable not found' &&
            echo 'To run on ARM device or emulator:' &&
            echo 'qemu-arm-static -L /usr/arm-linux-gnueabihf build-arm/tests/xuanyu_tests'
        "
    else
        echo "Running native tests in Docker container..."
        docker run "${DOCKER_ARGS[@]}" "$IMAGE_NAME" bash -c "
            ./scripts/build.sh --clean &&
            echo '=== Running all tests ===' &&
            cd build && ./tests/xuanyu_tests
        "
    fi
elif [[ -n "$COMMAND" ]]; then
    echo "Running command in Docker container: $COMMAND"
    docker run "${DOCKER_ARGS[@]}" "$IMAGE_NAME" bash -c "$COMMAND"
else
    echo "Starting interactive Docker container..."
    docker run "${DOCKER_ARGS[@]}" "$IMAGE_NAME"
fi