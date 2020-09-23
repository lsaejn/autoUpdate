#pragma once
#include <QThread>
#include <functional>
#include "AppUtility.h"

using TaskFunc = std::function<void()>;

class TaskThread : public QThread
{
    Q_OBJECT
public:
    TaskThread(QObject* parent, TaskFunc t)
        :QThread(parent),
        func_(std::move(t))
    {
        qDebug() << "TaskThread  constructed";
    }

    ~TaskThread()
    {
        qDebug() << "TaskThread  deleted";
    }

signals:
    void TaskFinished();
protected:
    void run()
    {
        func_();
    }
private:
    TaskFunc func_;
};
