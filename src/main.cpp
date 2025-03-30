#include <QApplication>
#include <git2.h>

int main(int argc, char* argv[])
{
    qDebug() << "Starting";

    git_libgit2_init();

    QApplication app(argc, argv);
    int result = app.exec();
    qDebug() << "Exiting:" << result;
    return result;
}
