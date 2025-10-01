#!/bin/bash
set -e

# 打印环境信息
echo "=== XuanYu Development Environment ==="
echo "GCC Version: $(gcc --version | head -n1)"
echo "CMake Version: $(cmake --version | head -n1)"
echo "====================================="

# 如果有参数，执行传入的命令
if [ $# -gt 0 ]; then
    exec "$@"
else
    # 默认启动bash
    exec /bin/bash
fi