// recvthread.h
#pragma once

#include <QThread>
#include <sys/socket.h>
#include <QString>
#include <QDebug>

#include "message.h" // Message2 定义在这里？

class RecvThread : public QThread {
    Q_OBJECT
public:
    explicit RecvThread(QObject *parent = nullptr)
        : QThread(parent) {}

    void setSocket(int socketFd) { clientSocket = socketFd; }
    void setGroupName(const QString &group) { groupName = group; }

signals:
    void messageReceived(const QString &message);

protected:
    void run() override {
        if (clientSocket == -1) {
            qDebug("clientSocket 未初始化");
            return;
        }

        while (true) {
            Message2 resp = {};
            size_t respSize = sizeof(MessageHeader) + sizeof(resp.payload.chat);
            char *rbuf = reinterpret_cast<char*>(&resp);
            size_t recvd = 0;
            while (recvd < respSize) {
                ssize_t n = recv(clientSocket, rbuf + recvd, respSize - recvd, 0);
                if (n <= 0) {
                    qDebug("接受消息失败或连接断开");
                    return;
                }
                recvd += n;
            }

            if (resp.header.type != MSG_CHAT) {
                qDebug("非聊天信息类型: %d", resp.header.type);
                continue;
            }

            if (QString(resp.header.receiver) == groupName) {
                QString message = resp.payload.chat.content;
                qDebug("接收到消息:%s", message.toStdString().c_str());
                emit messageReceived(message);
            } else {
                qDebug("接收到非当前群组消息");
            }
        }
    }

private:
    int clientSocket = -1;
    QString groupName;
};
