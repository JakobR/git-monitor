#include "editrepodialog.h"
#include "ui_editrepodialog.h"

EditRepoDialog::EditRepoDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EditRepoDialog)
{
    ui->setupUi(this);
}

EditRepoDialog::~EditRepoDialog()
{
    delete ui;
}

void EditRepoDialog::prepare(RepoSettings* repo)
{
    // TODO: set window title and fields
}

void EditRepoDialog::on_pathBrowseButton_clicked()
{

}
