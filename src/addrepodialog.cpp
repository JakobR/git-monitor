#include "addrepodialog.h"
#include "ui_addrepodialog.h"

AddRepoDialog::AddRepoDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddRepoDialog)
{
    ui->setupUi(this);
}

AddRepoDialog::~AddRepoDialog()
{
    delete ui;
}
