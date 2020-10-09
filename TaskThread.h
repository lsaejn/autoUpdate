#pragma once
#include <QThread>
#include <functional>
#include "AppUtility.h"



class TaskThread : public QThread
{
    Q_OBJECT
public:
    using TaskFunc = std::function<void()>;

    TaskThread(QObject* parent, TaskFunc t)
        :QThread(parent),
        func_(std::move(t))
    {
        qDebug() << "TaskThread  constructed";
    }

    ~TaskThread()
    {
        requestInterruption();
        quit();
        qDebug() << "TaskThread  deleted";
    }

signals:
    void TaskFinished();
protected:
    void run()
    {
        if(func_)
            func_();
    }
private:
    TaskFunc func_;
};


#include <atomic>

/*
* 关闭所有程序是因为可能控件正在被使用
*/
class SetupThread : public QThread
{
    Q_OBJECT
public:
    using TaskFunc = std::function<void()>;

    SetupThread(QObject* parent, const QString& path)
        :QThread(parent),
        localFilePath_(path)
    {
        hasInstance = true;
        qDebug() << "TaskThread  constructed";
    }

    ~SetupThread()
    {
        requestInterruption();
        quit();
        wait();
        if (func_)
            func_();
        qDebug() << "TaskThread  deleted";
        hasInstance = false;
    }

    static bool HasInstance() { return hasInstance; }

signals:
    void TaskFinished();
protected:
    void run()
    {
        QString path(localFilePath_);
        std::wstring u16AppPath = path.toStdWString();

        auto folder = GetExeFolderW();
        folder += L"..\\";
        folder = L"/S -PATH=\"" + folder + L"\"";

        SHELLEXECUTEINFO ShExecInfo = { 0 };
        ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        ShExecInfo.hwnd = NULL;
        ShExecInfo.lpVerb = L"open";
        ShExecInfo.hInstApp = NULL;
        ShExecInfo.lpFile = u16AppPath.c_str();
        ShExecInfo.lpParameters = folder.c_str();
        ShExecInfo.lpDirectory = NULL;
        ShExecInfo.nShow = SW_NORMAL;

        ShellExecuteExW(&ShExecInfo);
        if (ShExecInfo.hProcess)
        {
            while (!isInterruptionRequested())
            {
                if (WAIT_OBJECT_0 == WaitForSingleObject(ShExecInfo.hProcess, 1000))
                {
                    qDebug() << "setup finished ";
                    break;
                }
                else
                {
                    qWarning() << "WaitForSingleObject error, error code is: " << GetLastError();
                }
            }
            CloseHandle(ShExecInfo.hProcess);
        }
        else
        {
            qDebug() << "failed to create process";
        }
    }
private:
    TaskFunc func_;
    QString localFilePath_;
    static std::atomic<bool> hasInstance;
};
