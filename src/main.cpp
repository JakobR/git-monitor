#include <QApplication>

int main(int argc, char* argv[])
{
    qDebug() << "Starting";

    QApplication app(argc, argv);
    int result = app.exec();
    qDebug() << "Exiting:" << result;
    return result;
}
