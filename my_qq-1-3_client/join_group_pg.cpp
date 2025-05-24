#include "join_group_pg.h"
#include "ui_join_group_pg.h"
#include <QMessageBox>

join_group_pg::join_group_pg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::join_group_pg)
{
    ui->setupUi(this);
    this->setWindowTitle("欢迎注册");
    // 设置窗口为模态
    setModal(true);
    // 绑定"取消"按钮
    connect(ui->cancel_bt, &QPushButton::clicked, this, &join_group_pg::reject);

    // 绑定"注册"按钮
    connect(ui->create_bt, &QPushButton::clicked, [=]() {
        if (ui->group_name_j->text().isEmpty()) {
            QMessageBox::warning(this, "输入错误", "用户名和密码不能为空");
            return;
        }
        accept();  // 关闭窗口并返回 QDialog::Accepted
    });
}

join_group_pg::~join_group_pg()
{
    delete ui;
}
QString join_group_pg::_get_gropu_name_j()
{
    return ui->group_name_j->text();
}

