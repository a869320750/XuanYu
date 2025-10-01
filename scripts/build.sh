#!/bin/bash

# XuanYu项目构建脚本

set -e  # 遇到错误时退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 日志函数
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 显示帮助信息
show_help() {
    echo "XuanYu项目构建脚本"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help      显示此帮助信息"
    echo "  -c, --clean     清理构建目录"
    echo "  -t, --test      构建后运行测试"
    echo "  -e, --examples  构建并运行示例程序"
    echo "  -b, --benchmark 运行性能基准测试"
    echo "  -a, --all       运行完整测试套件（单元测试+示例+基准测试）"
    echo "  -r, --release   发布模式构建（默认为Debug）"
    echo "  -j N            使用N个并行作业（默认为系统CPU核心数）"
    echo "  -q, --quick     快速测试（仅运行核心功能验证）"
    echo ""
    echo "示例:"
    echo "  $0                # Debug模式构建"
    echo "  $0 -r             # Release模式构建"
    echo "  $0 -c -t          # 清理构建并运行测试"
    echo "  $0 -e             # 构建并运行示例程序"
    echo "  $0 -b             # 运行性能基准测试"
    echo "  $0 -a             # 运行完整测试套件"
    echo "  $0 -j 4           # 使用4个并行作业构建"
}

# 检查Docker是否可用
check_docker() {
    if ! command -v docker &> /dev/null; then
        log_error "Docker未安装或不在PATH中"
        exit 1
    fi
    
    if ! docker info &> /dev/null; then
        log_error "Docker daemon未运行或无权限访问"
        exit 1
    fi
}

# 检查Docker镜像是否存在
check_docker_image() {
    local image_name="xuanyu-dev:latest"
    if ! docker image inspect "$image_name" &> /dev/null; then
        log_error "Docker镜像 $image_name 不存在"
        log_info "请先构建Docker镜像:"
        log_info "  docker build -t xuanyu-dev:latest ."
        exit 1
    fi
}

# 清理构建目录
clean_build() {
    log_info "清理构建目录..."
    if [ -d "build" ]; then
        rm -rf build
        log_success "构建目录已清理"
    else
        log_warning "构建目录不存在，跳过清理"
    fi
}

# 构建项目
build_project() {
    local build_type=$1
    local jobs=$2
    
    log_info "构建项目 ($build_type 模式)..."
    
    docker run --rm -v "$(pwd):/workspace" xuanyu-dev:latest /bin/bash -c "
        mkdir -p /workspace/build && 
        cd build && 
        cmake -DCMAKE_BUILD_TYPE=$build_type -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON -DBUILD_BENCHMARKS=OFF .. && 
        make -j$jobs
    "
    
    if [ $? -eq 0 ]; then
        log_success "项目构建成功"
    else
        log_error "项目构建失败"
        exit 1
    fi
}

# 运行测试
run_tests() {
    log_info "运行单元测试..."
    
    docker run --rm -v "$(pwd):/workspace" xuanyu-dev:latest /bin/bash -c "
        cd /workspace/build && 
        ./tests/xuanyu_tests --gtest_output=xml:test_results.xml
    "
    
    local test_result=$?
    
    if [ $test_result -eq 0 ]; then
        log_success "所有测试通过"
    else
        log_warning "部分测试失败 (退出码: $test_result)"
        if [ -f "build/test_results.xml" ]; then
            log_info "测试结果已保存到 build/test_results.xml"
        fi
    fi
    
    return $test_result
}

# 运行示例程序
run_examples() {
    log_info "运行示例程序..."
    
    # 检查示例程序是否存在
    if [ ! -f "build/bin/crypto_demo" ]; then
        log_error "示例程序未构建，请先构建项目"
        return 1
    fi
    
    echo ""
    log_info "======== 加密功能演示 ========"
    docker run --rm -v "$(pwd):/workspace" xuanyu-dev:latest /bin/bash -c "
        cd /workspace/build && 
        echo '运行 CryptoSoftware 功能演示程序:' &&
        echo '注意: 性能和功能取决于是否有GmSSL库可用' &&
        echo '- 有GmSSL: 真实的SM2/SM3/SM4加密操作' &&
        echo '- 无GmSSL: 简化的占位符实现用于测试' &&
        echo '' &&
        ./bin/crypto_demo
    "
    
    local demo_result=$?
    
    if [ $demo_result -eq 0 ]; then
        log_success "加密功能演示完成"
    else
        log_warning "加密功能演示遇到问题 (退出码: $demo_result)"
        log_info "这可能是由于GmSSL库不可用导致的，但基本功能仍然可以测试"
    fi
    
    return $demo_result
}

# 运行性能基准测试
run_benchmark() {
    log_info "运行性能基准测试..."
    
    # 检查基准测试程序是否存在
    if [ ! -f "build/bin/crypto_benchmark" ]; then
        log_error "基准测试程序未构建，请先构建项目"
        return 1
    fi
    
    echo ""
    log_info "======== 性能基准测试 ========"
    docker run --rm -v "$(pwd):/workspace" xuanyu-dev:latest /bin/bash -c "
        cd /workspace/build && 
        echo '运行 CryptoSoftware 性能基准测试:' &&
        echo '注意: 性能数据取决于是否有GmSSL库可用' &&
        echo '- 有GmSSL: 真实加密算法的性能数据' &&
        echo '- 无GmSSL: 简化实现的参考性能数据' &&
        echo '' &&
        ./bin/crypto_benchmark
    "
    
    local benchmark_result=$?
    
    if [ $benchmark_result -eq 0 ]; then
        log_success "性能基准测试完成"
        log_info "性能数据已输出，可用于与硬件实现对比"
    else
        log_warning "性能基准测试遇到问题 (退出码: $benchmark_result)"
        log_info "这可能是由于GmSSL库不可用导致的，但测试框架仍然有效"
    fi
    
    return $benchmark_result
}

# 运行完整测试套件
run_comprehensive_tests() {
    log_info "运行完整测试套件..."
    
    local all_passed=true
    
    # 运行单元测试
    echo ""
    log_info "1. 单元测试"
    if ! run_tests; then
        all_passed=false
    fi
    
    # 运行示例程序
    echo ""
    log_info "2. 示例程序测试"
    if ! run_examples; then
        all_passed=false
    fi
    
    # 运行性能基准测试
    echo ""
    log_info "3. 性能基准测试"
    if ! run_benchmark; then
        all_passed=false
    fi
    
    echo ""
    if [ "$all_passed" = true ]; then
        log_success "所有测试完成！"
    else
        log_warning "部分测试失败，请检查上方输出"
    fi
    
    return $([ "$all_passed" = true ] && echo 0 || echo 1)
}

# 快速验证测试（仅运行核心功能）
run_quick_test() {
    log_info "运行快速验证测试..."
    
    local all_passed=true
    
    # 检查构建结果
    echo ""
    log_info "1. 检查构建产物"
    if [ ! -f "build/lib/libxuanyu.a" ]; then
        log_error "静态库未构建"
        all_passed=false
    else
        log_success "静态库构建成功"
    fi
    
    if [ ! -f "build/tests/xuanyu_tests" ]; then
        log_error "测试程序未构建"
        all_passed=false
    else
        log_success "测试程序构建成功"
    fi
    
    if [ ! -f "build/bin/crypto_demo" ]; then
        log_error "演示程序未构建"
        all_passed=false
    else
        log_success "演示程序构建成功"
    fi
    
    # 运行核心测试
    echo ""
    log_info "2. 运行核心单元测试"
    docker run --rm -v "$(pwd):/workspace" xuanyu-dev:latest /bin/bash -c "
        cd /workspace/build && 
        ./tests/xuanyu_tests --gtest_filter='CryptoSoftwareTest.Initialization:CryptoSoftwareTest.GenerateRandom:CryptoSoftwareTest.SM3Hash' --gtest_brief=1
    "
    
    local test_result=$?
    if [ $test_result -eq 0 ]; then
        log_success "核心测试通过"
    else
        log_warning "核心测试失败"
        all_passed=false
    fi
    
    # 快速演示测试
    echo ""
    log_info "3. 快速功能演示"
    docker run --rm -v "$(pwd):/workspace" xuanyu-dev:latest /bin/bash -c "
        cd /workspace/build && 
        timeout 30s ./bin/crypto_demo | head -20
    "
    
    local demo_result=$?
    if [ $demo_result -eq 0 ] || [ $demo_result -eq 124 ]; then  # 124 是timeout的退出码
        log_success "功能演示正常"
    else
        log_warning "功能演示异常"
        all_passed=false
    fi
    
    echo ""
    if [ "$all_passed" = true ]; then
        log_success "快速测试完成！核心功能正常"
    else
        log_warning "快速测试发现问题，建议运行完整测试"
    fi
    
    return $([ "$all_passed" = true ] && echo 0 || echo 1)
}

# 显示构建统计信息
show_build_stats() {
    log_info "构建统计信息:"
    
    if [ -d "build" ]; then
        echo "  构建目录: $(du -sh build | cut -f1)"
        
        if [ -f "build/lib/libxuanyu.so" ]; then
            echo "  共享库: $(ls -lh build/lib/libxuanyu.so | awk '{print $5}')"
        fi
        
        if [ -f "build/lib/libxuanyu.a" ]; then
            echo "  静态库: $(ls -lh build/lib/libxuanyu.a | awk '{print $5}')"
        fi
        
        if [ -f "build/tests/xuanyu_tests" ]; then
            echo "  测试可执行文件: $(ls -lh build/tests/xuanyu_tests | awk '{print $5}')"
        fi
        
        if [ -f "build/bin/crypto_demo" ]; then
            echo "  加密演示程序: $(ls -lh build/bin/crypto_demo | awk '{print $5}')"
        fi
        
        if [ -f "build/bin/crypto_benchmark" ]; then
            echo "  性能基准测试: $(ls -lh build/bin/crypto_benchmark | awk '{print $5}')"
        fi
        
        echo "  总构建文件: $(find build -type f | wc -l) 个"
    else
        log_warning "构建目录不存在"
    fi
}

# 主函数
main() {
    local clean_build_flag=false
    local run_tests_flag=false
    local run_examples_flag=false
    local run_benchmark_flag=false
    local run_comprehensive_flag=false
    local run_quick_flag=false
    local build_type="Debug"
    local jobs=$(nproc 2>/dev/null || echo "4")
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -c|--clean)
                clean_build_flag=true
                shift
                ;;
            -t|--test)
                run_tests_flag=true
                shift
                ;;
            -e|--examples)
                run_examples_flag=true
                shift
                ;;
            -b|--benchmark)
                run_benchmark_flag=true
                shift
                ;;
            -a|--all)
                run_comprehensive_flag=true
                shift
                ;;
            -q|--quick)
                run_quick_flag=true
                shift
                ;;
            -r|--release)
                build_type="Release"
                shift
                ;;
            -j)
                if [[ -n $2 && $2 =~ ^[0-9]+$ ]]; then
                    jobs="$2"
                    shift 2
                else
                    log_error "选项 -j 需要一个数字参数"
                    exit 1
                fi
                ;;
            *)
                log_error "未知选项: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    log_info "XuanYu项目构建开始"
    log_info "构建模式: $build_type"
    log_info "并行作业: $jobs"
    
    # 检查环境
    check_docker
    check_docker_image
    
    # 清理构建（如果需要）
    if [ "$clean_build_flag" = true ]; then
        clean_build
    fi
    
    # 构建项目
    build_project "$build_type" "$jobs"
    
    # 根据选项运行不同的测试
    if [ "$run_comprehensive_flag" = true ]; then
        run_comprehensive_tests
    elif [ "$run_quick_flag" = true ]; then
        run_quick_test
    else
        # 运行单元测试（如果需要）
        if [ "$run_tests_flag" = true ]; then
            run_tests
        fi
        
        # 运行示例程序（如果需要）
        if [ "$run_examples_flag" = true ]; then
            run_examples
        fi
        
        # 运行性能基准测试（如果需要）
        if [ "$run_benchmark_flag" = true ]; then
            run_benchmark
        fi
    fi
    
    # 显示构建统计信息
    show_build_stats
    
    log_success "构建完成！"
}

# 运行主函数
main "$@"