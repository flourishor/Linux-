# 使用 Ubuntu 作为基础镜像
FROM ubuntu:20.04

# 设置非交互式前端
ENV DEBIAN_FRONTEND=noninteractive

# 安装必需的工具和依赖项，并添加 CMake 的 APT 源
RUN apt-get update && \
    apt-get install -y \
    software-properties-common \
    build-essential \
    wget \
    libboost-all-dev \
    default-libmysqlclient-dev \
    libmysqlclient-dev \
    tzdata && \
    ln -fs /usr/share/zoneinfo/Asia/Shanghai /etc/localtime && \
    dpkg-reconfigure -f noninteractive tzdata && \
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.sh | bash && \
    apt-get install -y cmake && \
    rm -rf /var/lib/apt/lists/*

# 创建工作目录
WORKDIR /app

# 单独复制 CMakeLists.txt 以利用缓存
COPY CMakeLists.txt ./
# 如果你有其他依赖文件，也可以单独复制
# COPY dependencies.txt ./

# 可以在这里执行一些仅依赖 CMakeLists.txt 的操作
# RUN cmake . 

# 复制项目的其余文件
COPY . .

# 使用 CMake 构建项目
RUN cmake . && make && ls -l my_qq_1_3_server && chmod +x my_qq_1_3_server

# 暴露服务端口
EXPOSE 12345

# 使用绝对路径来指定可执行文件
CMD ["/app/my_qq_1_3_server"]
