
#include "register_page.h"
#include "ui_register_page.h"
#include <QMessageBox>

register_page::register_page(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::register_page)
{
    ui->setupUi(this);
    // 设置窗口为模态
    this->setWindowTitle("欢迎注册");
    setModal(true);
    // 绑定"取消"按钮
    connect(ui->cancel_button, &QPushButton::clicked, this, &register_page::reject);

    // 绑定"注册"按钮
    connect(ui->register_button_real, &QPushButton::clicked, [=]() {
        if (ui->enter_user_name->text().isEmpty() || ui->enter_user_password->text().isEmpty()) {
            QMessageBox::warning(this, "输入错误", "用户名和密码不能为空");
            return;
        }
        accept();  // 关闭窗口并返回 QDialog::Accepted
    });
}

register_page::~register_page()
{
    delete ui;
}

//void register_page::on_register_button_real_clicked()
//{

//}
QString register_page::getUsername() const
{
    return ui->enter_user_name->text();
}

QString register_page::getPassword() const
{
    return ui->enter_user_password->text();
}

