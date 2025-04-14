#pragma once

#include <QDialog>
#include "reposettings.h"

namespace Ui {
class EditRepoDialog;
}

class RepoSettings;

class EditRepoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditRepoDialog(QWidget *parent = nullptr);
    ~EditRepoDialog();

    // Prepare dialog for editing the given repo.
    // If the given pointer is null, prepare for adding a new repo.
    void prepare(RepoSettings* repo);

    void setValues(RepoSettings& repo);
    RepoSettings values() const;

public slots:
    void accept() override;

private slots:
    void on_pathBrowseButton_clicked();

private:
    Ui::EditRepoDialog *ui;
};
