#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "base/promise.h"
#include "base/coroutine.h"

#include <QThreadPool>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QThread::currentThread()->setObjectName("MainThread");


    q::makePromise<int>([](auto resolve, auto reject) {
        _dbg() << "resolve";
        resolve(23);
    }).then([](int i){
        _dbg() << "ok" << i;
    });

    _dbg() << "start";
}

MainWindow::~MainWindow()
{
    delete ui;
}
