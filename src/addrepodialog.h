#ifndef ADDREPODIALOG_H
#define ADDREPODIALOG_H

#include <QDialog>

namespace Ui {
class AddRepoDialog;
}

class AddRepoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddRepoDialog(QWidget *parent = nullptr);
    ~AddRepoDialog();

private:
    Ui::AddRepoDialog *ui;
};

#endif // ADDREPODIALOG_H
