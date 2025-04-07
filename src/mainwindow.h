#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "editrepodialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_addRepoButton_clicked();

private:
    void closeEvent(QCloseEvent* event) override;

    void readSettings();
    void writeSettings();

private:
    Ui::MainWindow* ui;
    EditRepoDialog* editRepoDialog = nullptr;
};

#endif // MAINWINDOW_H
