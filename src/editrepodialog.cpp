#include "editrepodialog.h"
#include "ui_editrepodialog.h"
#include <QFileDialog>

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
    if (!repo) {
        // Adding a new repository
        this->setWindowTitle(tr("Add Repository"));
        RepoSettings defaultSettings;
        setValues(defaultSettings);
    }
    else {
        // Editing the given repo
        this->setWindowTitle(tr("Edit Repository"));
        setValues(*repo);
    }
}

void EditRepoDialog::setValues(RepoSettings& repo)
{
    ui->pathEdit->setText(repo.path);
    ui->warnOnUncommittedCheckBox->setChecked(repo.warnOnUncommittedChanges);
    ui->warnOnUnpushedCheckBox->setChecked(repo.warnOnUnpushedCommits);
    ui->warnOnUnmergedCheckBox->setChecked(repo.warnOnUnmergedCommits);
    ui->warnOnUnfetchedCheckBox->setChecked(repo.warnOnUnfetchedCommits);
}

RepoSettings EditRepoDialog::values() const
{
    RepoSettings rs;
    rs.path = ui->pathEdit->text();
    rs.warnOnUncommittedChanges = ui->warnOnUncommittedCheckBox->isChecked();
    rs.warnOnUnpushedCommits = ui->warnOnUnpushedCheckBox->isChecked();
    rs.warnOnUnmergedCommits = ui->warnOnUnmergedCheckBox->isChecked();
    rs.warnOnUnfetchedCommits = ui->warnOnUnfetchedCheckBox->isChecked();
    return rs;
}

void EditRepoDialog::on_pathBrowseButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Git Repository"), ui->pathEdit->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isNull())
        return;  // the user pressed 'Cancel'
    ui->pathEdit->setText(dir);
}
