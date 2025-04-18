#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "repotablemodel.h"
#include "settings.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);
    m_ui->statusBar->hide();

    connect(m_ui->quitButton, &QPushButton::clicked, qApp, &QCoreApplication::quit);

    m_repoTableModel = new RepoTableModel(this);
    m_sortFilterModel = new QSortFilterProxyModel(this);
    m_sortFilterModel->setSourceModel(m_repoTableModel);
    m_ui->repoTableView->setModel(m_sortFilterModel);

    auto* header = m_ui->repoTableView->horizontalHeader();
    header->setSectionResizeMode(RepoTableModel::Column::Path, QHeaderView::Stretch);
    header->setSectionResizeMode(RepoTableModel::Column::Status, QHeaderView::Fixed);
    header->resizeSection(RepoTableModel::Column::Status, 120);
    header->setSortIndicator(RepoTableModel::Column::Path, Qt::SortOrder::AscendingOrder);

    readSettings();
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::setRepoManager(RepoManager* repoManager)
{
    m_repoTableModel->setRepoManager(repoManager);
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
    m_ui->repoTableView->horizontalHeader()->restoreState(settings.value(Settings::MainWindow::TableHeaderState).toByteArray());
    m_sortFilterModel->invalidate();
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.setValue(Settings::MainWindow::Geometry, saveGeometry());
    settings.setValue(Settings::MainWindow::TableHeaderState, m_ui->repoTableView->horizontalHeader()->saveState());
}

void MainWindow::on_addRepoButton_clicked()
{
    if (!m_editRepoDialog)
        m_editRepoDialog = new EditRepoDialog(this);
    m_editRepoDialog->prepare(nullptr);

    int result = m_editRepoDialog->exec();
    if (!result)
        return;  // dialog was cancelled

    RepoSettings rs = m_editRepoDialog->values();
    m_repoTableModel->addRepo(std::move(rs));
}
