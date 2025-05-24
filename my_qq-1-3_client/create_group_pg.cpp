#include "create_group_pg.h"
#include "ui_create_group_pg.h"

create_group_pg::create_group_pg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::create_group_pg)
{
    ui->setupUi(this);
    this->setWindowTitle("欢迎创建");
    // 设置窗口为模态
    setModal(true);
    // 绑定"取消"按钮
    connect(ui->cancel_bt, &QPushButton::clicked, this, &create_group_pg::reject);

    // 绑定"注册"按钮
    connect(ui->create_bt, &QPushButton::clicked, [=]() {
        if (ui->gropu_name->text().isEmpty()) {
            QMessageBox::warning(this, "输入错误", "用户名和密码不能为空");
            return;
        }
        accept();  // 关闭窗口并返回 QDialog::Accepted
    });
}

create_group_pg::~create_group_pg()
{
    delete ui;
}

QString create_group_pg::_get_gropu_name()
{
    return ui->gropu_name->text();
}

