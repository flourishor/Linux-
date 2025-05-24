#ifndef REGISTER_PAGE_H
#define REGISTER_PAGE_H

#include <QDialog>

namespace Ui {
class register_page;
}

class register_page : public QDialog
{
    Q_OBJECT

public:
    explicit register_page(QWidget *parent = nullptr);
    ~register_page();
    QString getUsername() const;
    QString getPassword() const;

private slots:
//    void on_register_button_real_clicked();


private:
    Ui::register_page *ui;
};

#endif // REGISTER_PAGE_H
