#include <mysql/mysql.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <vector>
// #include <algorithm>
#include <pthread.h>
#include "ThreadPool.h"
#include "Message.h"
#include "ConnectionPool.h"
#define CLIENT_SIZE 1024

// MYSQL *sql;

std::vector<int> clients;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;//互斥锁
ThreadPool pool(4); //线程池
ConnectionPool Cpool("localhost","root","p@ssw0rd","qq_db_1_3",3306, 8);//连接池

void client_store_chatMessage_func(Message *msg);

void client_register_func(Message *msg);

void client_login_func(Message *msg);

void client_lookup_func(Message *msg);

void client_reg_group_func(Message *msg);

void client_join_group_func(Message *msg);

int main() {
    // // 1. 创建子进程
    // pid_t pid = fork();
    // if (pid < 0) {
    //     perror("fork error");
    //     exit(1);
    // } else if (pid > 0) {
    //     // 父进程退出
    //     exit(0);
    // }
    //
    // // 2. 子进程创建新会话
    // if (setsid() < 0) {
    //     perror("setsid error");
    //     exit(1);
    // }
    //
    // // 3. 重定向标准输入输出至 /dev/null
    // freopen("/dev/null", "r", stdin);
    // freopen("/dev/null", "w", stdout);
    // freopen("/dev/null", "w", stderr);
    // printf("Server:\n");
    //
    // 4. 启动服务监听主逻辑
    std::cout << "Server started!\n" << std::endl;
    // printf("Server (daemon) started\n");

    // 5. 创建socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == -1) {
        perror("socket error");
        exit(1);
    }
    // 允许端口重用，避免 "Address already in use" 问题
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    std::cout << "socket success!\n" << std::endl;

    // 6. 创建地址协议族
    struct sockaddr_in addr;
    addr.sin_family = AF_INET; // ipv4
    // addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    addr.sin_port = htons(10086);

    // 7. 绑定
    int r = bind(serverSocket, (sockaddr *) &addr, sizeof(addr));
    if (r == -1) {
        perror("bind");
        exit(1);
    }
    std::cout << "bind success!\n" << std::endl;

    // 8. 监听
    r = listen(serverSocket, 10);
    if (r == -1) {
        perror("listen");
        exit(1);
    }
    std::cout << "listen success!\n" << std::endl;

    // 9. 创建epoll
    int epld = epoll_create1(0);
    if (epld < 0) {
        perror("epoll");
        exit(1);
    }
    std::cout << "epoll success!\n" << std::endl;

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = serverSocket;

    int ret = epoll_ctl(epld, EPOLL_CTL_ADD, serverSocket, &ev);
    if (ret < 0) {
        perror("epoll_ctl");
        exit(1);
    }
    std::cout << "epoll_ctl success!\n" << std::endl;
    // // 6. 初始化数据库
    // sql = mysql_init(NULL);
    // if (sql == NULL) {
    //     fprintf(stderr, "mysql_init failed: %s\n", mysql_error(sql));
    //     return -1;
    // }
    // // 7. 连接数据库
    // if (mysql_real_connect(sql, "localhost", "root", "p@ssw0rd", "qq_db_1_3", 1, NULL, 0) == NULL) {
    //     fprintf(stderr, "mysql_real_connect failed: %s\n", mysql_error(sql));
    //     mysql_close(sql);
    //     return -1;
    // }
    // printf("数据库连接成功\n");

    while (1) {
        struct epoll_event evs[CLIENT_SIZE];
        //-1 是等待时间，单位为毫秒。-1 表示 epoll_wait 将无限期地等待，直到有事件发生。
        int n = epoll_wait(epld, evs, CLIENT_SIZE, -1);
        //  使用epoll_wait函数等待事件的发生         epld: epoll实例的文件描述符         evs: 事件数组，用于存储发生的事件         CLIENT_SIZE: 事件数组的大小，即最多可以存储的事件数量         -1: 等待时间为无限，直到有事件发生才返回
        if (n < 0) {
            perror("epoll_wait");
            exit(1);
        }

        // 遍历所有的事件
        for (int i = 0; i < n; i++) {
            // 获取当前事件的文件描述符
            int fd = evs[i].data.fd;
            // 如果是监听的fd收到消息，那么表示有客户端进行连接
            if (fd == serverSocket) {
                //  检查文件描述符 fd 是否等于服务器套接字 serverSocket
                // 定义客户端地址结构体并初始化为0
                struct sockaddr_in cAddr = {0};
                // 定义地址长度
                socklen_t len = sizeof(cAddr);
                // 接受客户端连接，返回客户端的文件描述符
                int client_sockfd = accept(serverSocket, (sockaddr *) &cAddr, &len);
                // 如果接受失败，打印错误信息并退出
                if (client_sockfd < 0) {
                    perror("accept");
                    exit(1);
                }
                // 设置新客户端的事件为可读事件
                ev.events = EPOLLIN;
                // 设置新客户端的文件描述符
                ev.data.fd = client_sockfd;
                // 将新客户端的文件描述符添加到epoll事件列表中
                ret = epoll_ctl(epld, EPOLL_CTL_ADD, client_sockfd, &ev);
                // 如果添加失败，打印错误信息并退出
                if (ret < 0) {
                    perror("epoll_ctl");
                    exit(1);
                }
                // 打印客户端连接信息
                std::cout << "收到客户端连接" <<clients.size()<< std::endl;
                // printf("Client %lu connected...\n", clients.size());
                // 将新客户端的文件描述符添加到客户端列表中
                pthread_mutex_lock(&clients_mutex);
                clients.push_back(client_sockfd);
                pthread_mutex_unlock(&clients_mutex);
            } else // 如果是客户端有消息
            {
                Message msg;
                int r = recv(fd, &msg, sizeof(msg), 0);
                if (r <= 0) {
                    // `r == 0` 表示对端关闭连接，`r < 0` 说明出错
                    if (r < 0)
                        perror("recv error");
                    std::cout<<"客户端："<<fd<<"断开连接";
                    // printf("客户端 %d 断开连接\n", fd);
                    pthread_mutex_lock(&clients_mutex);
                    clients.erase(std::remove(clients.begin(), clients.end(), fd), clients.end());
                    pthread_mutex_unlock(&clients_mutex);
                    epoll_ctl(epld, EPOLL_CTL_DEL, fd, nullptr);
                    close(fd);
                    continue;
                }

                switch (msg.header.type) {
                    case MSG_LOGIN: {
                        // 接收登录请求
                        std::cout << "准备接收登录请求..." << std::endl;
                        Message *msg_copy = new Message(msg);
                        // 储存 socket fd 到 receiver
                        snprintf(msg_copy->header.receiver,
                                 sizeof(msg_copy->header.receiver), "%d", fd);
                        // 提交到线程池
                        pool.enqueue([msg_copy]() {
                            client_login_func(msg_copy);
                            // delete msg_copy;  // 确保释放
                        });
                        break;
                        // pthread_t pd = 0;
                        // Message *msg_copy = new Message(msg);
                        // snprintf(msg_copy->header.receiver, sizeof(msg_copy->header.receiver), "%d", fd);
                        // pthread_create(&pd, nullptr, client_login_func, msg_copy);
                        // pthread_detach(pd);
                        std::cout<<"用户"<<msg_copy->payload.auth.username<<"请求登陆";
                        // printf("用户 %s 请求登录\n", msg_copy->payload.auth.username);
                        break;
                    }
                    case MSG_CHAT: {
                        Message *msg_copy = new Message(msg);
                        printf("用户 %s 发送消息: %s\n", msg.header.sender, msg.payload.chat.content);
                        // 7. 发送给所有其他客户端
                        pthread_mutex_lock(&clients_mutex);
                        for (int client_fd: clients) {
                            if (client_fd != fd) {
                                send(client_fd, &msg, sizeof(msg), 0);
                            }
                        }
                        pthread_mutex_unlock(&clients_mutex);
                        // 提交到线程池
                        pool.enqueue([msg_copy]() {
                            client_store_chatMessage_func(msg_copy);
                        });
                        printf("转发给了%d个用户\n", clients.size() - 1);
                        break;
                    }
                    case MSG_LOGOUT: {
                        close(fd);
                        epoll_ctl(epld, EPOLL_CTL_DEL, fd, nullptr);
                        clients.erase(std::remove(clients.begin(), clients.end(), fd), clients.end());
                        printf("用户 %s 退出\n", msg.header.sender);
                        break;
                    }
                    case MSG_REGISTER: {
                        printf("client注册");
                        // pthread_t pd = 0;
                        Message *msg_copy = new Message(msg);
                        snprintf(msg_copy->header.receiver, sizeof(msg_copy->header.receiver), "%d", fd);
                        // pthread_create(&pd, NULL, client_register_func, msg_copy);
                        // pthread_detach(pd);
                        pool.enqueue([msg_copy]() {
                            client_register_func(msg_copy);
                        });
                        printf("account:%s:password:%s\n", msg_copy->payload.auth.username,
                               msg_copy->payload.auth.password);
                        break;
                    }
                    case MSG_LOOKUP: {
                        // printf("客户查询历史消息");
                        // pthread_t pd = 0;
                        Message *msg_copy = new Message(msg);
                        snprintf(msg_copy->header.receiver, sizeof(msg_copy->header.receiver), "%d", fd);
                        // pthread_create(&pd, nullptr, client_lookup_func, msg_copy);
                        // pthread_detach(pd);
                        pool.enqueue([msg_copy]() {
                            client_lookup_func(msg_copy);
                        });
                        printf("用户请求查询群聊%s的聊天记录\n", msg_copy->payload.lookupReq.groupName);
                        break;
                    }
                    case MSG_CREATE_GROUP: {
                        printf("客户创建群聊\n");
                        // pthread_t pd = 0;
                        Message *msg_copy = new Message(msg);
                        snprintf(msg_copy->header.receiver, sizeof(msg_copy->header.receiver), "%d", fd);
                        // pthread_create(&pd, NULL, client_reg_group_func, msg_copy);
                        // pthread_detach(pd);
                        pool.enqueue([msg_copy]() {
                            client_reg_group_func(msg_copy);
                        });
                        printf("群聊名：%s", msg_copy->payload.group.groupName);
                        break;
                    }
                    case MSG_JOIN_GROUP: {
                        printf("用户加入群聊\n");
                        // pthread_t pd = 0;
                        Message *msg_copy = new Message(msg);
                        snprintf(msg_copy->header.receiver, sizeof(msg_copy->header.receiver), "%d", fd);
                        // pthread_create(&pd, NULL, client_join_group_func, msg_copy);
                        // pthread_detach(pd);
                        pool.enqueue([msg_copy]() {
                            client_join_group_func(msg_copy);
                        });
                        printf("群聊名:%s\n", msg_copy->payload.group.groupName);
                        break;
                    }
                }
            }
        }
    }
    close(serverSocket);
    // mysql_close(sql);
    pthread_mutex_destroy(&clients_mutex);
    return 0;
}
void client_store_chatMessage_func(Message *msg) {
    ConnectionGuard guard(Cpool);        // 从连接池中获取连接
    MYSQL* sql = guard.get();

    // 构造 SQL 语句
    char query[2048];
    snprintf(query, sizeof(query),
        "INSERT INTO group_messages (group_id, sender_id, message) "
        "VALUES ("
        " (SELECT group_id FROM qq_groups WHERE group_name = '%s'),"
        " (SELECT uid FROM users WHERE username = '%s'),"
        " '%s');",
        msg->header.receiver, msg->header.sender, msg->payload.chat.content
    );

    // 执行 SQL
    if (mysql_query(sql, query)) {
        fprintf(stderr, "Insert group message failed: %s\n", mysql_error(sql));
    } else {
        printf("Message stored successfully.\n");
    }
    delete msg;
    // mysql_close(sql);
}
void client_register_func(Message *msg) {
    // Message *msg = static_cast<Message *>(arg);
    ConnectionGuard guard(Cpool);
    MYSQL* sql = guard.get();
    // 安全截断用户名和密码
    msg->payload.auth.username[sizeof(msg->payload.auth.username) - 1] = '\0';
    msg->payload.auth.password[sizeof(msg->payload.auth.password) - 1] = '\0';

    printf("开始注册用户\n");
    printf("用户名: %s, 密码: %s\n", msg->payload.auth.username, msg->payload.auth.password);

    // 准备数据库插入语句
    MYSQL_STMT *stmt = mysql_stmt_init(sql);
    const char *query = "INSERT INTO users (username, password) VALUES (?, ?)";
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "Prepare failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        delete msg;
        return;
    }

    // 绑定参数
    MYSQL_BIND bind[2] = {};
    unsigned long len1 = strlen(msg->payload.auth.username);
    unsigned long len2 = strlen(msg->payload.auth.password);

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = msg->payload.auth.username;
    bind[0].buffer_length = sizeof(msg->payload.auth.username);
    bind[0].length = &len1;

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = msg->payload.auth.password;
    bind[1].buffer_length = sizeof(msg->payload.auth.password);
    bind[1].length = &len2;

    mysql_stmt_bind_param(stmt, bind);

    // 执行插入
    bool success = true;
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "Execute failed: %s\n", mysql_stmt_error(stmt));
        success = false;
    }

    mysql_stmt_close(stmt);
    if (success)
        printf("注册成功\n");
    else
        printf("注册失败\n");
    // // 构造响应消息
    // Message response = {};
    // response.header.type = MSG_REGISTER;
    // strncpy(response.header.receiver, msg->header.sender, sizeof(response.header.receiver) - 1);
    // strncpy(response.header.sender, "server", sizeof(response.header.sender) - 1);
    //
    // // 复用 auth 字段传注册结果（true/false）
    // snprintf(response.payload.auth.username, sizeof(response.payload.auth.username), success ? "success" : "fail");
    // response.payload.auth.password[0] = '\0';  // 避免误用
    //
    // // 发送响应
    // int client_fd = atoi(msg->header.receiver);  // header.receiver 存的是客户端 socket fd 的字符串
    // if (send(client_fd, &response, sizeof(MessageHeader) + sizeof(AuthPayload), 0) < 0) {
    //     perror("send register response error");
    // }

    delete msg;
    // mysql_close(sql);
    return;
}

void client_login_func(Message *msg) {
    std::cout<<"开始用户注册服务";
    // Message *msg = static_cast<Message *>(arg);
    ConnectionGuard guard(Cpool);
    MYSQL* sql = guard.get();
    // 安全截断用户名和密码
    msg->payload.auth.username[sizeof(msg->payload.auth.username) - 1] = '\0';
    msg->payload.auth.password[sizeof(msg->payload.auth.password) - 1] = '\0';

    if (!sql) {
        fprintf(stderr, "数据库未初始化\n");
        delete msg;
        return;
    }

    // 一、验证用户名和密码
    MYSQL_STMT *stmt = mysql_stmt_init(sql);
    const char *passwd_query = "SELECT password FROM users WHERE username = ?";
    if (mysql_stmt_prepare(stmt, passwd_query, strlen(passwd_query))) {
        fprintf(stderr, "Prepare failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        delete msg;
        return;
    }
    // 绑定用户名参数
    MYSQL_BIND bind_param = {};
    unsigned long userLen = strlen(msg->payload.auth.username);
    bind_param.buffer_type = MYSQL_TYPE_STRING;
    bind_param.buffer = msg->payload.auth.username;
    bind_param.buffer_length = sizeof(msg->payload.auth.username);
    bind_param.length = &userLen;
    mysql_stmt_bind_param(stmt, &bind_param);

    // 执行查询
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "Execute failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        delete msg;
        return;
    }

    // 绑定结果
    MYSQL_BIND bind_result = {};
    char storedPwd[64];
    unsigned long pwdLen = 0;
    bind_result.buffer_type = MYSQL_TYPE_STRING;
    bind_result.buffer = storedPwd;
    bind_result.buffer_length = sizeof(storedPwd);
    bind_result.length = &pwdLen;
    mysql_stmt_bind_result(stmt, &bind_result);

    mysql_stmt_store_result(stmt);
    bool loginSuccess = false;
    if (mysql_stmt_num_rows(stmt) == 1) {
        mysql_stmt_fetch(stmt);
        storedPwd[pwdLen] = '\0';
        // 检查密码
        if (strcmp(storedPwd, msg->payload.auth.password) == 0) {
            loginSuccess = true;
            printf("用户 %s 登录成功\n", msg->payload.auth.username);
        } else {
            printf("用户 %s 登录失败，密码错误\n", msg->payload.auth.username);
        }
    } else {
        printf("用户 %s 不存在\n", msg->payload.auth.username);
    }
    mysql_stmt_close(stmt);
    //创建回复Message
    Message resp = {};
    resp.header.type = MSG_LOGIN_RES;
    // printf("%s\n",msg->payload.auth.username);
    // 二、填充 LoginResponsePayload
    resp.header.type = MSG_LOGIN_RES;
    resp.payload.loginResp.loginSuccess = loginSuccess;
    resp.payload.loginResp.groupCount = 0;

    if (loginSuccess) {
        // 查询该用户已加入的群组
        printf("开始查询用户已加入的群组\n");
        char grpQuery[256];
        // char uname_test[64];
        // strcpy(uname_test,msg->payload.auth.username);
        // printf("%s\n",uname_test);
        // printf("%s\n", msg->payload.auth.username);
        snprintf(grpQuery, sizeof(grpQuery),
                 "SELECT qq_groups.group_name FROM users "
                 "JOIN group_members ON users.uid = group_members.uid "
                 "JOIN qq_groups ON group_members.group_id = qq_groups.group_id "
                 "WHERE users.username = '%s'",
                 msg->payload.auth.username);
        if (mysql_query(sql, grpQuery) == 0) {
            // printf("查询成功\n");
            MYSQL_RES *res = mysql_store_result(sql);
            if (res) {
                // printf("准备计算结果\n");
                int idx = 0;
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res)) && idx < MAX_GROUPS) {
                    strncpy(
                        resp.payload.loginResp.groupNames[idx],
                        row[0],
                        sizeof(resp.payload.loginResp.groupNames[idx]) - 1
                    );
                    resp.payload.loginResp.groupNames[idx][sizeof(resp.payload.loginResp.groupNames[idx]) - 1] = '\0';
                    idx++;
                }
                resp.payload.loginResp.groupCount = idx;
                mysql_free_result(res);
            }
        }
        std::cout<<"查询到用户已加入"<<resp.payload.loginResp.groupCount<<"个群组";
        // printf("查询到用户已加入%d个群组\n", resp.payload.loginResp.groupCount);
    }
    // printf("用户当前未加入群组");
    // 三、发送响应
    int client_fd = atoi(msg->header.receiver);
    size_t send_size = sizeof(MessageHeader) + sizeof(resp.payload.loginResp);
    if (send(client_fd, &resp, send_size, 0) < 0) {
        perror("send login response error");
    }

    delete msg;
    // mysql_close(sql);
    return;
}


void client_lookup_func(Message *msg) {
    ConnectionGuard guard(Cpool);
    MYSQL* sql = guard.get();
    // 接收客户端请求
    // Message *msg = static_cast<Message *>(arg);

    // 解析客户端 socket fd
    int client_fd = atoi(msg->header.receiver);

    // 安全截断群组名
    msg->payload.lookupReq.groupName[sizeof(msg->payload.lookupReq.groupName) - 1] = '\0';
    int maxCount = msg->payload.lookupReq.maxCount;
    if (maxCount <= 0 || maxCount > MAX_HISTORY_ITEMS) {
        maxCount = MAX_HISTORY_ITEMS;
    }
    Message resp = {};
    // 准备响应类型
    resp.header.type = MSG_LOOKUP_RESPONSE;
    resp.payload.lookupRsp.count = 0;

    // 防注入转义
    char escGroup[128];
    mysql_real_escape_string(sql,
                             escGroup,
                             msg->payload.lookupReq.groupName,
                             strlen(msg->payload.lookupReq.groupName));

    // 构造查询语句（按时间升序，并限制条数）
    char query[512];
    snprintf(query, sizeof(query),
             "SELECT users.username, group_messages.message "
             "FROM users "
             "JOIN group_messages ON users.uid = group_messages.sender_id "
             "JOIN qq_groups ON group_messages.group_id = qq_groups.group_id "
             "WHERE qq_groups.group_name = '%s' "
             // "ORDER BY group_messages.timestamp ASC "
             "LIMIT %d",
             escGroup, maxCount);

    if (mysql_query(sql, query) != 0) {
        fprintf(stderr, "数据库查询失败: %s\n", mysql_error(sql));
    } else {
        MYSQL_RES *res = mysql_store_result(sql);
        if (!res) {
            fprintf(stderr, "获取查询结果失败: %s\n", mysql_error(sql));
        } else {
            MYSQL_ROW row;
            int idx = 0;
            while ((row = mysql_fetch_row(res)) && idx < maxCount) {
                // 格式化 "用户名: 消息内容"
                // snprintf(resp.payload.lookupRsp.messages[idx],
                //          sizeof(resp.payload.lookupRsp.messages[idx]),
                //          "%s: %s", row[0], row[1]);
                snprintf(resp.payload.lookupRsp.messages[idx],
                            sizeof(resp.payload.lookupRsp.messages[idx]),
                            "%s", row[1]);               
                idx++;
            }
            resp.payload.lookupRsp.count = idx;
            mysql_free_result(res);
        }
    }

    // 发送响应：只发送消息头 + LookupResponsePayload
    size_t respSize = sizeof(MessageHeader)
                      + sizeof(resp.payload.lookupRsp);
    if (send(client_fd, &resp, respSize, 0) < 0) {
        perror("send lookup response error");
    }

    // 清理
    delete msg;
    // mysql_close(sql);
    return;
}

void client_reg_group_func(Message *msg) {
    ConnectionGuard guard(Cpool);
    MYSQL* sql = guard.get();
    // Message *msg = static_cast<Message *>(arg);
    // 安全截断
    msg->header.sender[sizeof(msg->header.sender) - 1] = '\0';
    msg->payload.group.groupName[sizeof(msg->payload.group.groupName) - 1] = '\0';
    const char *username = msg->header.sender;
    const char *groupName = msg->payload.group.groupName;
    printf("开始注册群组\n");
    printf("用户名: %s, 群组名: %s\n", msg->header.sender, msg->payload.group.groupName);

    // 插入群组表
    char insertGroupQuery[512];
    snprintf(insertGroupQuery, sizeof(insertGroupQuery),
             "INSERT INTO qq_groups (group_name, owner_id) "
             "VALUES ('%s', (SELECT uid FROM users WHERE username = '%s'))",
             groupName, username);

    if (mysql_query(sql, insertGroupQuery) != 0) {
        fprintf(stderr, "插入群组失败: %s\n", mysql_error(sql));
        delete msg;
        return;
    }

    // 插入成员表（添加创建者为管理员）
    char insertMemberQuery[1024];
    snprintf(insertMemberQuery, sizeof(insertMemberQuery),
             "INSERT INTO group_members (group_id, uid, is_admin) "
             "VALUES ("
             "  (SELECT group_id FROM qq_groups WHERE group_name = '%s' AND owner_id = (SELECT uid FROM users WHERE username = '%s')), "
             "  (SELECT uid FROM users WHERE username = '%s'), "
             "  1)",
             groupName, username, username);

    if (mysql_query(sql, insertMemberQuery) != 0) {
        fprintf(stderr, "插入群成员失败: %s\n", mysql_error(sql));
        delete msg;
        return;
    }

    printf("群组 %s 注册成功，创建者 %s 已设置为管理员\n", groupName, username);

    delete msg;
    // mysql_close(sql);
    return;
}

void client_join_group_func(Message *msg) {
    ConnectionGuard guard(Cpool);
    MYSQL* sql = guard.get();
    // Message *msg = static_cast<Message *>(arg);
    // 安全截断
    msg->header.sender[sizeof(msg->header.sender) - 1] = '\0';
    msg->payload.group.groupName[sizeof(msg->payload.group.groupName) - 1] = '\0';
    const char *username = msg->header.sender;
    const char *groupName = msg->payload.group.groupName;
    printf("用户请求加入群组\n");
    printf("用户名: %s, 群组名: %s\n", msg->header.sender, msg->payload.group.groupName);

    // 插入成员表（添加创建者为管理员）
    char insertMemberQuery[1024];
    snprintf(insertMemberQuery, sizeof(insertMemberQuery),
             "INSERT INTO group_members (group_id, uid, is_admin) "
             "VALUES ("
             "  (SELECT group_id FROM qq_groups WHERE group_name = '%s'), "
             "  (SELECT uid FROM users WHERE username = '%s'), "
             "  0)",
             groupName, username);

    if (mysql_query(sql, insertMemberQuery) != 0) {
        fprintf(stderr, "插入群成员失败: %s\n", mysql_error(sql));
        delete msg;
        return;
    }

    printf("加入群组 %s 成功\n", groupName, username);

    delete msg;
    // mysql_close(sql);
    return;
}
