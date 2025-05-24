#include "widget.h"
#include "ui_widget.h"
#include "message.h"
#include "QPlainTextEdit"
#include "QTimer"
#include "QScrollBar"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
//#include <thread>
#include <sys/socket.h>
#include <create_group_pg.h>
#include <join_group_pg.h>
#include "recvthread.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this);
    //    init(clientSocket);
    this->setWindowTitle("群聊:");
    qDebug("Widget初始化成功");
}
void Widget::init(int clientSocket,char* username)
{
    strcpy(this->username,username);
    this->clientSocket = clientSocket;
//    // 创建一个线程，用于执行接收消息的函数recv_func
//    std::thread RecvMsg(&Widget::_recv_func, this);
//    // 将创建的线程分离，使其独立运行，不与主线程同步
//    RecvMsg.detach();
    _startRecvThread();
}
Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{
    QString message_ui = ui->send_message->toPlainText();
    ui->send_message->clear();
//    qDebug("Message:%s", message_ui.toStdString().c_str());
    _create_message_row(message_ui);
    std::string username = this->username;
    std::string message_his = (username + ":");
    message_his += message_ui.toStdString();
    Message2 msg;
    msg.header.type = MSG_CHAT;
    strncpy(msg.header.sender, username.c_str(), sizeof(msg.header.sender) - 1);
    strncpy(msg.payload.chat.content,message_his.c_str(),sizeof(msg.payload.chat.content)-1);
    strncpy(msg.header.receiver,this->groupName,sizeof(msg.header.receiver)-1);
    // 发送请求
    {
        const char *buf = reinterpret_cast<const char*>(&msg);
        size_t toSend = sizeof(Message);
        size_t sent = 0;
        while (sent < toSend) {
            ssize_t n = send(clientSocket, buf + sent, toSend - sent, 0);
            if (n <= 0) {
                qDebug("群聊CHAT：发送失败或连接已断开");
                return;
            }
            sent += n;
        }
    }
    qDebug("用户向群聊%s中发送了信息%s",this->groupName,message_his.c_str());
//    strncpy(msg.header.receiver,
//    Message msg;
//    msg.msg_type = CHAT;
//    std::string username = this->username;
//    std::string message_his = (username + ":");
//    message_his+=message_ui.toStdString();
//    strncpy(msg.username, username.c_str(), sizeof(msg.username) - 1);
//    strncpy(msg.data, message_his.c_str(), sizeof(msg.data) - 1);
//    qDebug("用户：%s发送了: %s", msg.username, msg.data);
//    send(clientSocket, &msg, sizeof(msg), 0);
}
void Widget::_remove_message_row(int index)
{
    QWidget *contentWidget = ui->message_body->widget();
    if (!contentWidget)
        return;

    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(contentWidget->layout());
    if (!layout || index < 0 || index >= layout->count())
        return;

    QLayoutItem *item = layout->takeAt(index);
    if (item)
    {
        QWidget *widget = item->widget();
        if (widget)
        {
            widget->deleteLater();
        }
        delete item;
    }
}
void Widget::_delete_all_message()
{
    QWidget *contentWidget = ui->message_body->widget();
    if (!contentWidget) return;  // 没有内容区域，直接返回

    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(contentWidget->layout());
    if (!layout) return;  // 没有布局，直接返回

    while (layout->count() > 0) {
        QLayoutItem *item = layout->takeAt(0);
        if (item) {
            delete item->widget(); // 删除控件
            delete item;  // 删除布局项
        }
    }
}
void Widget::_delete_group_list()
{
    QWidget *contentWidget = ui->group_table->widget();
    if (!contentWidget) return;  // 没有内容区域，直接返回

    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(contentWidget->layout());
    if (!layout) return;  // 没有布局，直接返回

    while (layout->count() > 0) {
        QLayoutItem *item = layout->takeAt(0);
        if (item) {
            delete item->widget(); // 删除控件
            delete item;  // 删除布局项
        }
    }
}
void Widget:: _set_groupName(QString groupname)
{
    strncpy(this->groupName,groupname.toStdString().c_str(),sizeof(this->groupName)-1);
    this->setWindowTitle(groupname.toStdString().c_str());
}
//void Widget::_lookUp_gru_mes(QString text)
//{
//    Message msg = {};

//}
//bool Widget::_recv_all(int socket, void* buffer, int length)
//{
//    int total = 0;
//    char* ptr = (char*)buffer;
//    while (total < length) {
//        int bytes = recv(socket, ptr + total, length - total, 0);
//        if (bytes <= 0) return false;
//        total += bytes;
//    }
//    return true;
//}
void Widget::_startRecvThread() {
    auto *recvThread = new RecvThread(this);
    recvThread->setSocket(this->clientSocket);
    recvThread->setGroupName(this->groupName);

    connect(recvThread, &RecvThread::messageReceived, this, [this](const QString &msg) {
        this->_create_message_row(msg);
    });

    recvThread->start();  // 启动线程，自动执行 run()
}


void Widget::_create_message_row(QString text)
{
    QWidget *contentWidget = ui->message_body->widget();
    if (!contentWidget)
    {
        contentWidget = new QWidget();
        ui->message_body->setWidget(contentWidget);
        ui->message_body->setWidgetResizable(true);
    }

    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(contentWidget->layout());
    if (!layout)
    {
        layout = new QVBoxLayout(contentWidget);
        layout->setAlignment(Qt::AlignTop);
        contentWidget->setLayout(layout);
    }

    QPlainTextEdit *plainTextEdit = new QPlainTextEdit(contentWidget);
    plainTextEdit->setPlainText(text);
    plainTextEdit->setFixedHeight(60);
    plainTextEdit->setReadOnly(true);
    plainTextEdit->setFrameStyle(QFrame::NoFrame); // 去掉边框
    plainTextEdit->setStyleSheet("background-color: #E8E8E8; border-radius: 5px; padding: 5px;");

    layout->addWidget(plainTextEdit);

    // 滚动到底部（更优雅的方法）
    QMetaObject::invokeMethod(ui->message_body->verticalScrollBar(), "setValue",
                              Qt::QueuedConnection,
                              Q_ARG(int, ui->message_body->verticalScrollBar()->maximum()));
}

void Widget::_recv_func()
{
    if (clientSocket == -1)
    {
        qDebug("clientSocket 未初始化");
        return;
    }
    while(1)
    {
        // 接收响应：只读 MessageHeader + LookupResponsePayload
        Message2 resp = {};
        size_t respSize = sizeof(MessageHeader)
                        + sizeof(resp.payload.chat);
        {
            char *rbuf = reinterpret_cast<char*>(&resp);
            size_t recvd = 0;
            while (recvd < respSize) {
                ssize_t n = recv(clientSocket, rbuf + recvd, respSize - recvd, 0);
                if (n <= 0) {
                    qDebug("接受其他用户信息时失败");
                    break;
                }
                recvd += n;
            }
        }

        // 检查响应类型
        if (resp.header.type != MSG_CHAT) {
            qDebug("非聊天信息类型: %d", resp.header.type);
            return;
        }
        else if(strcpy(resp.header.receiver,this->groupName))
        {
            QString message = resp.payload.chat.content;
            qDebug("接收到消息:%s",message.toStdString().c_str());
            QMetaObject::invokeMethod(this, [this, message]()
                                      { _create_message_row(message); }, Qt::QueuedConnection);
        }
        else
        {
            qDebug("接收到非当前群组消息");
        }
    }
//    if (clientSocket == -1)
//    {
//        qDebug("clientSocket 未初始化");
//        return;
//    }
//    while (true)
//    {
//        Message msg;
//        int r = recv(clientSocket, &msg, sizeof(msg), 0);
//        if (r <= 0)
//        {
//            qDebug("网络错误，服务器可能已断开(in_widget接受信息失败(如登陆成功请忽略))");
//            return;
//        }
//        if(msg.msg_type == CHAT)
//        {
//            QString message = msg.data;
//            qDebug("接收到消息:%s",message.toStdString().c_str());
//            QMetaObject::invokeMethod(this, [this, message]()
//                                      { _create_message_row(message); }, Qt::QueuedConnection);
//        }
//    }
//    ::close(clientSocket); // ✅ 关闭连接，防止内存泄漏
}

void Widget::on_logoutButton_clicked()
{
    emit this->back(); // 发送信号
    Message2 msg = {};
    msg.header.type = MSG_LOGOUT;
    strncpy(msg.header.sender, this->username, sizeof(msg.header.sender) - 1);
    // 发送请求
    {
        const char *buf = reinterpret_cast<const char*>(&msg);
        size_t toSend = sizeof(Message);
        size_t sent = 0;
        while (sent < toSend) {
            ssize_t n = send(clientSocket, buf + sent, toSend - sent, 0);
            if (n <= 0) {
                qDebug("退出登陆：发送失败或连接已断开");
                return;
            }
            sent += n;
        }
    }
//    Message msg = {};
//    msg.msg_type = LOGOUT;
//    send(clientSocket, &msg, sizeof(msg), 0);
}
void Widget::_create_group_row(QString text)
{
    qDebug("在home_page中_create_group_row：%s",text.toStdString().c_str());
    QWidget *contentWidget = ui->group_table->widget();
    if (!contentWidget)
    {
        contentWidget = new QWidget();
        ui->group_table->setWidget(contentWidget);
        ui->group_table->setWidgetResizable(true); // 确保 QScrollArea 内部区域不会随内容变化
    }

    // 确保内容区域有布局（QVBoxLayout），并且消息从上到下排列
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(contentWidget->layout());
    if (!layout)
    {
        layout = new QVBoxLayout(contentWidget);
        layout->setAlignment(Qt::AlignTop); // 保证消息从上到下排列
        contentWidget->setLayout(layout);
    }

    // 创建新的消息框
    QPushButton *pushButton = new QPushButton(contentWidget);
    //    plainTextEdit->setPlaceholderText(text);
    pushButton->setText(text);
    pushButton->setFixedHeight(60); // 让每条消息高度固定
    layout->addWidget(pushButton);
    // 为每个按钮添加点击事件
    connect(pushButton, &QPushButton::clicked, this, [this, text]() {
        // 当按钮被点击时，调用这个槽函数
        qDebug("用户点击了群聊%s",text.toStdString().c_str());
        // 在这里添加你想要执行的操作
        this->_delete_all_message();
        this->_generate_history_message(text);
        this->_set_groupName(text);

    });
    // **核心：滚动到底部**
    QTimer::singleShot(100, this, [this]()
                       { ui->group_table->verticalScrollBar()->setValue(
                             ui->group_table->verticalScrollBar()->maximum()); });
}
void Widget::_generate_history_message(QString groupName)
{
    // 构造查询历史消息请求
    Message2 msg = {};
    msg.header.type = MSG_LOOKUP;
    // 填充发送者用户名（从 widget 的 private username 字段）
    strncpy(msg.header.sender, this->username, sizeof(msg.header.sender) - 1);
    // 填充查询参数：群组名 + 最大条数
    QByteArray ba = groupName.toUtf8();
    strncpy(msg.payload.lookupReq.groupName, ba.constData(),
            sizeof(msg.payload.lookupReq.groupName) - 1);
    msg.payload.lookupReq.maxCount = MAX_HISTORY_ITEMS;

    // 发送请求
    {
        const char *buf = reinterpret_cast<const char*>(&msg);
        size_t toSend = sizeof(Message);
        size_t sent = 0;
        while (sent < toSend) {
            ssize_t n = send(clientSocket, buf + sent, toSend - sent, 0);
            if (n <= 0) {
                qDebug("查询历史消息：发送失败或连接已断开");
                return;
            }
            sent += n;
        }
    }

    // 设置接收超时（可选）
    struct timeval tv {10, 0};
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // 接收响应：只读 MessageHeader + LookupResponsePayload
    Message2 rep = {};
    size_t respSize = sizeof(MessageHeader)
                    + sizeof(rep.payload.lookupRsp);
    {
        char *rbuf = reinterpret_cast<char*>(&rep);
        size_t recvd = 0;
        while (recvd < respSize) {
            ssize_t n = recv(clientSocket, rbuf + recvd, respSize - recvd, 0);
            if (n <= 0) {
                qDebug("查询历史消息失败或超时");
                return;
            }
            recvd += n;
        }
    }

    // 检查响应类型
    if (rep.header.type != MSG_LOOKUP_RESPONSE) {
        qDebug("收到非历史消息响应类型: %d", rep.header.type);
        return;
    }

    // 清空旧消息
    _delete_all_message();

    // 遍历并显示历史消息
    int cnt = rep.payload.lookupRsp.count;
    if (cnt < 0 || cnt > MAX_HISTORY_ITEMS) {
        qDebug("收到的历史消息条数异常：%d", cnt);
        return;
    }
    for (int i = 0; i < cnt; ++i) {
        const char *hist = rep.payload.lookupRsp.messages[i];
        if (hist && hist[0] != '\0') {
            qDebug("历史消息[%d]: %s", i, hist);
            _create_message_row(QString::fromUtf8(hist));
        }
    }
//    qDebug("开始生成历史消息。。。。。。");
//    Message msg3 = {};
//    msg3.msg_type = LOOKUP;
//    strcpy(msg3.group_name, groupName.toStdString().c_str());
//    send(clientSocket, &msg3, sizeof(msg3), 0);
//    Message msg4 = {};
//    qDebug("已发送，等待接受。。。。。。");

//    // 设置超时时间为10秒
//    struct timeval tv;
//    tv.tv_sec = 10;
//    tv.tv_usec = 0;
//    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

//    int r = recv(clientSocket, &msg4, sizeof(msg4), 0);
//    if (r <= 0) {
//        // 错误或服务器断开连接
//        qDebug("查询历史消息失败,请尝试手动刷新(in_widget接收历史信息失败)");
//        return;
//    }
//    qDebug("已接受。。。。。。");
//    if (msg4.history_count < 0 || msg4.history_count > 50) {
//        qDebug("收到的历史消息条数异常：%d", msg4.history_count);
//        return;
//    }
//    for (int i = 0; i < msg4.history_count; i++)
//    {
//        if (msg4.history_messages[i] == nullptr) {
//            qDebug("第 %d 条历史消息为空，跳过", i);
//            continue;
//        }
//        qDebug("历史信息有：%s", msg4.history_messages[i]);
//        _create_message_row(msg4.history_messages[i]);
//    }

}


void Widget::on_create_group_bt_clicked()
{
    create_group_pg c_g_dialog(this);
    if (c_g_dialog.exec() == QDialog::Accepted)
    {
        Message2 msg = {};
        msg.header.type = MSG_CREATE_GROUP;
        std::string gropuN = c_g_dialog._get_gropu_name().toStdString();
        strncpy(msg.payload.group.groupName, gropuN.c_str(), sizeof(msg.payload.group.groupName) - 1);
        std::string username = this->username;
        strncpy(msg.header.sender, username.c_str(), sizeof(msg.header.sender) - 1);
//        send(clientSocket, &msg, sizeof(msg), 0);
        // 发送请求
        {
            const char *buf = reinterpret_cast<const char*>(&msg);
            size_t toSend = sizeof(Message);
            size_t sent = 0;
            while (sent < toSend) {
                ssize_t n = send(clientSocket, buf + sent, toSend - sent, 0);
                if (n <= 0) {
                    qDebug("创建群聊：发送失败或连接已断开");
                    return;
                }
                sent += n;
            }
        }
        qDebug("用户创建了群聊：%s", msg.payload.group.groupName);
    }
}

void Widget::on_join_group_bt_clicked()
{
    join_group_pg j_g_dialog(this);
    if(j_g_dialog.exec()==QDialog::Accepted)
    {
        Message2 msg = {};
        msg.header.type = MSG_JOIN_GROUP;
        std::string groupN = j_g_dialog._get_gropu_name_j().toStdString();
        strncpy(msg.payload.group.groupName,groupN.c_str(),sizeof(msg.payload.group.groupName));
        std::string username = this->username;
        strncpy(msg.header.sender, username.c_str(), sizeof(msg.header.sender) - 1);
        // 发送请求
        {
            const char *buf = reinterpret_cast<const char*>(&msg);
            size_t toSend = sizeof(Message);
            size_t sent = 0;
            while (sent < toSend) {
                ssize_t n = send(clientSocket, buf + sent, toSend - sent, 0);
                if (n <= 0) {
                    qDebug("创建群聊：发送失败或连接已断开");
                    return;
                }
                sent += n;
            }
        }
        qDebug("用户加入群聊:%s",msg.payload.group.groupName);


    }
}

