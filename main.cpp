#include "mainwindow.h"
#include <QApplication>


#undef main         // 避免和SDL2的main重定义

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.showMaximized();

    return a.exec();
}
