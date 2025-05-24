#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#define BUFFER_SIZE 1024

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void init(int clientSocket,char* username);
    void _create_group_row(QString text);
    void _generate_history_message(QString groupName);
    void _remove_message_row(int index);
    void _delete_all_message();
    void _delete_group_list();
    void _set_groupName(QString groupname);
//    void _lookUp_gru_mes(QString text);
//    void _recv_all(int socket, void* buffer, int length);
    void _startRecvThread();
signals:
    void back();//定义一个信号
private slots:

    void on_pushButton_clicked();
    // void login();
    void on_logoutButton_clicked();

    void on_create_group_bt_clicked();

    void on_join_group_bt_clicked();

private:
    Ui::Widget *ui;
    int clientSocket;
//    void _send_func();
    void _recv_func();
    void _create_message_row(QString text);
    char username[32];//用户账户名
    char groupName[32];//当前群组名
};
#endif // WIDGET_H
