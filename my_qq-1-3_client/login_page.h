#ifndef LOGIN_PAGE_H
#define LOGIN_PAGE_H

#include <QWidget>
#include "widget.h"
#include <QMessageBox>

namespace Ui {
class login_page;
}

class login_page : public QWidget
{
    Q_OBJECT

public:
    explicit login_page(QWidget *parent = nullptr);
    ~login_page();
    void _init ();
    void _init_net();
    Widget *home_page = NULL;

private slots:
    void on_login_button_clicked();
    void on_register_button_clicked();

private:
    Ui::login_page *ui;
    int clientSocket;
};

#endif // LOGIN_PAGE_H
