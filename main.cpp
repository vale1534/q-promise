#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

#include <crtdbg.h>

int main(int argc, char *argv[])
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
