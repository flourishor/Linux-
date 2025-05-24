#include "login_page.h"
#include "ui_login_page.h"
#include "register_page.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <QDebug>
#include <string>
#include <vector>
#include "message.h"
login_page::login_page(QWidget *parent) : QWidget(parent),
                                          ui(new Ui::login_page)
{
    ui->setupUi(this);
    //    qDebug("hello");
    this->setWindowTitle("欢迎登陆");
    //    _init_net();
    _init_net();
    _init();
    qDebug("登陆界面连接上了服务端");
}

login_page::~login_page()
{
    delete ui;
}
void login_page::_init()
{
    home_page = new Widget();
    connect(this->home_page, &Widget::back, [=]()
            {
        this->home_page->hide();
        this->show();
        _init_net(); });
}
void login_page::_init_net()
{
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == -1)
    {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET; // ipv4
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(10086);
    // 定义一个整型变量r，用于存储connect函数的返回值
    int r = ::connect(clientSocket, (sockaddr *)&addr, sizeof(addr));
    // 判断connect函数的返回值是否为-1，-1表示连接失败
    if (r == -1)
    {
        // 如果连接失败，使用perror函数输出错误信息，"bind"是错误信息的提示前缀
        qDebug("网络错误");
        // 调用exit函数，参数1表示程序因错误退出
        exit(1);
    }
}
void login_page::on_login_button_clicked()
{
    // 构造并发送登录请求
    Message2 msg = {};
    msg.header.type = MSG_LOGIN;
    // 填充发送者用户名
    strncpy(msg.header.sender,
            ui->user_id->text().toUtf8().constData(),
            sizeof(msg.header.sender) - 1);
    // 填充用户凭证
    std::string uName     = ui->user_id->text().toStdString();
    std::string uPassword = ui->user_password->text().toStdString();
    if (uName.empty() || uPassword.empty()) {
        QMessageBox::warning(this, "!", "用户名或密码不能为空!");
        return;
    }
    if (uName.size() >= sizeof(msg.payload.auth.username) ||
        uPassword.size() >= sizeof(msg.payload.auth.password)) {
        QMessageBox::warning(this, "输入超限!", "用户名或密码长度不能超过63!");
        return;
    }
    strncpy(msg.payload.auth.username, uName.c_str(),
            sizeof(msg.payload.auth.username) - 1);
    strncpy(msg.payload.auth.password, uPassword.c_str(),
            sizeof(msg.payload.auth.password) - 1);

    // 发送整个 Message2 结构
    {
        const char *buf = reinterpret_cast<const char *>(&msg);
        size_t toSend   = sizeof(Message2);
        size_t sent     = 0;
        while (sent < toSend) {
            ssize_t n = send(clientSocket, buf + sent, toSend - sent, 0);
            if (n <= 0) {
                qDebug("发送失败，网络错误");
                return;
            }
            sent += n;
        }
    }

    // 接收服务器返回的 LoginResponsePayload
    Message2 rep = {};
    // 只读 MessageHeader + LoginResponsePayload
    size_t respSize = sizeof(MessageHeader)
                    + sizeof(rep.payload.loginResp);
    {
        char *rbuf      = reinterpret_cast<char *>(&rep);
        size_t received = 0;
        while (received < respSize) {
            ssize_t n = recv(clientSocket,
                             rbuf + received,
                             respSize - received,
                             0);
            if (n <= 0) {
                qDebug(n == 0 ? "服务器已断开连接" : "recv() 失败，网络错误");
                ::close(clientSocket);
                clientSocket = -1;
                return;
            }
            received += n;
        }
    }

    // 检查消息类型
    if (rep.header.type != MSG_LOGIN_RES) {
        QMessageBox::warning(this, "错误", "收到未知响应类型");
        return;
    }

    // 处理登录结果
    if (!rep.payload.loginResp.loginSuccess) {
        QMessageBox::information(this,
                                 "登录失败",
                                 "用户名或密码错误");
        return;
    }

    // 登录成功，初始化主界面
    if (!home_page) {
        home_page = new Widget();
    }
    char usernameC[64] = {0};
    strncpy(usernameC,
            ui->user_id->text().toUtf8().constData(),
            sizeof(usernameC) - 1);
    home_page->init(clientSocket, usernameC);

    // 清理遗留信息
    home_page->_delete_all_message();
    home_page->_delete_group_list();

    // 创建群组列表
    int groupCount = rep.payload.loginResp.groupCount;
    for (int i = 0; i < groupCount; ++i) {
        const char *grp = rep.payload.loginResp.groupNames[i];
        if (grp[0] != '\0') {
            home_page->_create_group_row(QString::fromUtf8(grp));
        }
    }

    // 加载首个群组的历史消息
    if (groupCount > 0) {
        QString firstGroup =
            QString::fromUtf8(rep.payload.loginResp.groupNames[0]);
        home_page->_generate_history_message(firstGroup);
        home_page->_set_groupName(firstGroup);
    }

    // 显示主界面，隐藏登录页
    home_page->show();
    this->hide();

    //    Message msg = {};
    //    msg.msg_type = LOGIN;

    //    std::string uName = ui->user_id->text().toStdString();
    //    std::string uPassword = ui->user_password->text().toStdString();
    //    if(uName.length()<=0||uPassword.length()<=0)
    //    {
    //        QMessageBox::warning(this,"!","用户名或密码不能为空!");
    //        return;
    //    }
    //    else if(uName.length()>50||uPassword.length()>50)
    //    {
    //        QMessageBox::warning(this,"输入超限!","用户名或密码长度不能超过50!");
    //        return;
    //    }
    //    snprintf(msg.username, sizeof(msg.username), "%s", uName.c_str());
    //    snprintf(msg.password, sizeof(msg.password), "%s", uPassword.c_str());

    //    // 发送数据，确保全部发送
    //    ssize_t bytes_sent = 0;
    //    while (bytes_sent < sizeof(msg)) {
    //        ssize_t ret = send(clientSocket, ((char*)&msg) + bytes_sent, sizeof(msg) - bytes_sent, 0);
    //        if (ret <= 0) {
    //            qDebug("发送失败，网络错误");
    //            return;
    //        }
    //        bytes_sent += ret;
    //    }

    //    // 接收服务器的响应
    //    Message msg2 = {};
    //    ssize_t bytes_received = 0;
    //    while (bytes_received < sizeof(msg2)) {
    //        ssize_t ret = recv(clientSocket, ((char*)&msg2) + bytes_received, sizeof(msg2) - bytes_received, 0);
    //        if (ret <= 0) {
    //            if (ret == 0) {
    //                qDebug("服务器已断开连接");
    //            } else {
    //                qDebug("recv() 失败，网络错误");
    //            }
    //            ::close(clientSocket);
    //            clientSocket = -1;
    //            return;
    //        }
    //        bytes_received += ret;
    //    }

    //    if (msg2.login_flag) {
    //        if (!home_page) {
    //            home_page = new Widget();
    //        }
    //        char username[32];
    //        strcpy(username,ui->user_id->text().toUtf8().constData());
    //        home_page->init(clientSocket,username);

    //        if (msg2.group_count < 0 ) {
    //            qDebug("收到的 group_count 数据异常: %d", msg2.group_count);
    //            return;
    //        }
    //        home_page->_delete_all_message();
    //        home_page->_delete_group_list();
    //        qDebug("清理遗留信息完成");
    //        for (int i = 0; i < msg2.group_count; i++) {
    //            if (msg2.group_names[i][0] == '\0') continue;
    //            qDebug("查询到已加入群组: %s", msg2.group_names[i]);
    //            home_page->_create_group_row(QString::fromUtf8(msg2.group_names[i]));
    //        }
    //        qDebug("创建群组列表完成");
    //        if (msg2.group_count > 0) {
    //            home_page->_generate_history_message(msg2.group_names[0]);
    //        }
    //        qDebug("查询历史信息完成");
    //        home_page->show();
    //        this->hide();
    //    } else {
    //        qDebug("用户名或密码错误");
    //        QMessageBox::warning(this, "输入错误", "用户名或密码错误");
    //        return;
    //    }
}

void login_page::on_register_button_clicked()
{
    // 创建注册窗口
    register_page regDialog(this);

    // 以模态方式运行注册窗口
    if (regDialog.exec() == QDialog::Accepted)
    {
        // 获取注册信息
        Message2 msg = {};
        msg.header.type = MSG_REGISTER;

        std::string uName = regDialog.getUsername().toStdString();
        std::string uPassword = regDialog.getPassword().toStdString(); // 获取密码而不是再次获取用户名

        // 复制用户名和密码到 payload.auth 中
        strncpy(msg.payload.auth.username, uName.c_str(), sizeof(msg.payload.auth.username) - 1);
        msg.payload.auth.username[sizeof(msg.payload.auth.username) - 1] = '\0';

        strncpy(msg.payload.auth.password, uPassword.c_str(), sizeof(msg.payload.auth.password) - 1);
        msg.payload.auth.password[sizeof(msg.payload.auth.password) - 1] = '\0';

//        // 将 socket fd 存入 sender 字段作为客户端唯一标识
//        snprintf(msg.header.sender, sizeof(msg.header.sender), "%d", clientSocket);

        qDebug("发送注册信息 -> 用户名: %s 密码: %s", msg.payload.auth.username, msg.payload.auth.password);

        // 发送注册请求
        send(clientSocket, &msg, sizeof(msg), 0);
//        // 接收响应：只读 MessageHeader + Auth
//        Message2 resp = {};
//        size_t respSize = sizeof(MessageHeader)
//                        + sizeof(resp.payload.auth);
//        {
//            char *rbuf = reinterpret_cast<char*>(&resp);
//            size_t recvd = 0;
//            while (recvd < respSize) {
//                ssize_t n = recv(clientSocket, rbuf + recvd, respSize - recvd, 0);
//                if (n <= 0) {
//                    qDebug("接受注册响应超时或失败");
//                    return;
//                }
//                recvd += n;
//            }
//        }
//        // 检查响应类型
//        if (resp.header.type != MSG_REGISTER) {
//            qDebug("收到非历史消息响应类型: %d", resp.header.type);
//            return;
//        }
//        if(resp.payload.auth.username=="success")
//        {
//            QMessageBox::information(this,"注册成功","请关闭当前页");
//        }
    }
//    // 创建注册窗口
//    register_page regDialog(this);

//    // 以模态方式运行注册窗口
//    if (regDialog.exec() == QDialog::Accepted)
//    {
//        // 获取注册信息
//        Message msg = {};
//        msg.msg_type = REGISTER;

//        std::string uName = regDialog.getUsername().toStdString();
//        std::string uPassword = regDialog.getUsername().toStdString();
//        strncpy(msg.username, uName.c_str(), sizeof(msg.username) - 1);
//        msg.username[sizeof(msg.username) - 1] = '\0';

//        strncpy(msg.password, uPassword.c_str(), sizeof(msg.password) - 1);
//        msg.password[sizeof(msg.password) - 1] = '\0';
//        qDebug("account:%s:password:%s", msg.username, msg.password);
//        // 发送注册请求
//        send(clientSocket, &msg, sizeof(msg), 0);
//    }
}
