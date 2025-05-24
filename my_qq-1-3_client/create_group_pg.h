#ifndef CREATE_GROUP_PG_H
#define CREATE_GROUP_PG_H

#include <QDialog>
#include <QMessageBox>

namespace Ui {
class create_group_pg;
}

class create_group_pg : public QDialog
{
    Q_OBJECT

public:
    explicit create_group_pg(QWidget *parent = nullptr);
    ~create_group_pg();
    QString _get_gropu_name();


private:
    Ui::create_group_pg *ui;
};

#endif // CREATE_GROUP_PG_H
