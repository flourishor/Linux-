#ifndef JOIN_GROUP_PG_H
#define JOIN_GROUP_PG_H

#include <QDialog>
#include <QMessageBox>

namespace Ui {
class join_group_pg;
}

class join_group_pg : public QDialog
{
    Q_OBJECT

public:
    explicit join_group_pg(QWidget *parent = nullptr);
    ~join_group_pg();
    QString _get_gropu_name_j();

private:
    Ui::join_group_pg *ui;
};

#endif // JOIN_GROUP_PG_H
