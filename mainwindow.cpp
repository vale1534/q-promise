#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "base/promise.h"
#include "base/coroutine.h"

#include <iostream>
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QThreadPool>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QThread::currentThread()->setObjectName("MainThread");

    auto pool = QThreadPool::globalInstance();

    auto promise = q::makePromise<int>([](auto resolve, auto reject) {
        _dbg() << "resolve";
        resolve(23);
    })
    .switchContext(pool)
    .then([](int i){
        _dbg() << "ok" << i << QThread::currentThread();
        return i + 1;
    })
    .switchContext()
    .then([](int i){
        _dbg() << "i1" << i << QThread::currentThread();
        return i + 1;
    })
    .switchContext(pool)
    .then([=](int i){
        _dbg() << "i2" << i << QThread::currentThread();

        QObject *obj = new QObject;
        connect(obj, &QObject::destroyed, [=]{
            _dbg() << "object destoryed";
        });
        obj->thread()->start();
        _dbg() << obj << obj->thread();
        return obj;
    })
    .then([](QObject *obj){
        _dbg() << obj << obj->thread();
        delete obj;
    });

    _dbg() << "start";
}

MainWindow::~MainWindow()
{
    delete ui;
}
