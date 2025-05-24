#include "ConnectionPool.h"
#include <iostream>

ConnectionPool::ConnectionPool(const std::string &host,
                               const std::string &user,
                               const std::string &passwd,
                               const std::string &db,
                               unsigned int port,
                               size_t poolSize)
    : host_(host), user_(user), passwd_(passwd),
      db_(db), port_(port), poolSize_(poolSize) {
    initPool();
}

ConnectionPool::~ConnectionPool() {
    std::lock_guard<std::mutex> lock(mtx_);
    while (!connections_.empty()) {
        MYSQL *conn = connections_.front();
        connections_.pop();
        mysql_close(conn);
    }
}

void ConnectionPool::initPool() {
    for (size_t i = 0; i < poolSize_; ++i) {
        MYSQL *conn = mysql_init(nullptr);
        if (!conn) {
            std::cerr << "mysql_init 失败\n";
            continue;
        }
        if (!mysql_real_connect(conn, "my_qq_mysql", "root", "p@ssw0rd", "qq_db_1_3", 3306, NULL, 0)) {
            std::cerr << "mysql_real_connect 失败: "
                    << mysql_error(conn) << "\n";
            mysql_close(conn);
            continue;
        }
        std::lock_guard<std::mutex> lock(mtx_);
        connections_.push(conn);
    }
}

MYSQL *ConnectionPool::getConnection() {
    std::unique_lock<std::mutex> lock(mtx_);
    // 等待直到有可用连接
    cv_.wait(lock, [this] { return !connections_.empty(); });
    MYSQL *conn = connections_.front();
    connections_.pop();
    return conn;
}

void ConnectionPool::releaseConnection(MYSQL *conn) { {
        std::lock_guard<std::mutex> lock(mtx_);
        connections_.push(conn);
    }
    // 通知一个等待线程
    cv_.notify_one();
}
