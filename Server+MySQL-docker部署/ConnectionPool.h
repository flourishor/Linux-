#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

class ConnectionPool {
public:
    // 初始化：传入数据库参数和池大小
    ConnectionPool(const std::string& host,
                   const std::string& user,
                   const std::string& passwd,
                   const std::string& db,
                   unsigned int port,
                   size_t poolSize);
    ~ConnectionPool();

    // 从池中获取一个连接，阻塞直到有可用连接
    MYSQL* getConnection();
    // 归还连接到池中
    void releaseConnection(MYSQL* conn);

private:
    void initPool();  // 内部调用，创建所有连接

    std::string host_, user_, passwd_, db_;
    unsigned int port_;
    size_t poolSize_;

    std::queue<MYSQL*> connections_;
    std::mutex            mtx_;
    std::condition_variable cv_;
};
class ConnectionGuard {
public:
    ConnectionGuard(ConnectionPool& pool)
        : pool_(pool), conn_(pool_.getConnection()) {}
    ~ConnectionGuard() {
        pool_.releaseConnection(conn_);
    }
    // 访问底层 MYSQL*
    MYSQL* get() { return conn_; }

private:
    ConnectionPool& pool_;
    MYSQL* conn_;
};

#endif // CONNECTION_POOL_H
