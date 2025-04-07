#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settings.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    readSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    writeSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::readSettings()
{
    QSettings settings;
    restoreGeometry(settings.value(Settings::MainWindow::Geometry).toByteArray());
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.setValue(Settings::MainWindow::Geometry, saveGeometry());
}

void MainWindow::on_addRepoButton_clicked()
{
    if (!editRepoDialog)
        editRepoDialog = new EditRepoDialog(this);
    editRepoDialog->prepare(nullptr);

    int result = editRepoDialog->exec();
    if (!result) {
        qDebug() << "cancelled";
    }

    qDebug() << "result: " << result;
}
