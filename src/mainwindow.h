#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSortFilterProxyModel>
#include "editrepodialog.h"
#include "repomanager.h"
#include "repotablemodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setRepoManager(RepoManager* repoManager);

private slots:
    void on_addRepoButton_clicked();

private:
    void closeEvent(QCloseEvent* event) override;

    void readSettings();
    void writeSettings();

private:
    Ui::MainWindow* m_ui = nullptr;
    EditRepoDialog* m_editRepoDialog = nullptr;
    RepoTableModel* m_repoTableModel = nullptr;
    QSortFilterProxyModel* m_sortFilterModel = nullptr;
};

#endif // MAINWINDOW_H
