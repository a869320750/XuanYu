# XuanYu高安全通信框架 - 开发环境
FROM ubuntu:22.04

LABEL maintainer="XuanYu Team <xuanyu@example.com>"
LABEL description="XuanYu Development and Testing Environment"

# 设置环境变量
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Shanghai
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

# 安装基础依赖
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    curl \
    wget \
    unzip \
    ca-certificates \
    tzdata \
    # C++开发工具
    gcc-11 \
    g++-11 \
    clang-14 \
    clang-tools-14 \
    libc++-14-dev \
    libc++abi-14-dev \
    # 构建工具
    ninja-build \
    make \
    automake \
    autoconf \
    libtool \
    # 调试和分析工具
    gdb \
    valgrind \
    perf-tools-unstable \
    # 静态分析工具
    cppcheck \
    clang-tidy-14 \
    # 网络工具
    netcat-openbsd \
    tcpdump \
    wireshark-common \
    # 其他工具
    vim \
    tree \
    htop \
    && rm -rf /var/lib/apt/lists/*

# 设置默认编译器
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 100 \
    && update-alternatives --install /usr/bin/clang clang /usr/bin/clang-14 100 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-14 100

# 安装GmSSL
WORKDIR /tmp
RUN git clone https://github.com/guanzhi/GmSSL.git \
    && cd GmSSL \
    && mkdir build && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make -j$(nproc) \
    && make install \
    && ldconfig \
    # 为 pkg-config 创建一个简单的 gmssl.pc，指向 /usr/local
    && mkdir -p /usr/local/lib/pkgconfig \
    && mkdir -p /usr/local/lib/pkgconfig \
    && true \
    && cd / && rm -rf /tmp/GmSSL

# 安装nlohmann/json
RUN git clone https://github.com/nlohmann/json.git \
    && cd json \
    && mkdir build && cd build \
    && cmake .. -DJSON_BuildTests=OFF \
    && make -j$(nproc) \
    && make install \
    && cd / && rm -rf /tmp/json

# 安装GoogleTest
RUN git clone https://github.com/google/googletest.git \
    && cd googletest \
    && mkdir build && cd build \
    && cmake .. -DBUILD_GMOCK=ON \
    && make -j$(nproc) \
    && make install \
    && cd / && rm -rf /tmp/googletest

# 安装Google Benchmark (可选)
RUN git clone https://github.com/google/benchmark.git \
    && cd benchmark \
    && cmake -E make_directory "build" \
    && cmake -E chdir "build" cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on -DCMAKE_BUILD_TYPE=Release ../ \
    && cmake --build "build" --config Release -j$(nproc) \
    && cmake --build "build" --config Release --target install \
    && cd / && rm -rf /tmp/benchmark

# 安装代码覆盖率工具
# 注意：`gcov` 不是独立包，通常随 gcc 提供；只安装 lcov（以及必要的工具）
RUN apt-get update && apt-get install -y \
    lcov \
    && rm -rf /var/lib/apt/lists/*

# 创建工作目录
WORKDIR /workspace
RUN mkdir -p /workspace/build

# 创建非root用户
RUN useradd -m -s /bin/bash xuanyu \
    && usermod -aG sudo xuanyu \
    && echo "xuanyu ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# 设置用户环境
USER xuanyu
WORKDIR /workspace

# 设置shell
SHELL ["/bin/bash", "-c"]

# 健康检查
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD which gcc && which cmake && which gmssl || exit 1

# 入口脚本
COPY docker/entrypoint.sh /usr/local/bin/
COPY docker/gmssl.pc /usr/local/lib/pkgconfig/gmssl.pc
USER root
RUN chmod +x /usr/local/bin/entrypoint.sh
USER xuanyu

ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["/bin/bash"]
