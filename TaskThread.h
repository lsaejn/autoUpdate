#pragma once
#include <QThread>
#include <QFile>
#include <QDir>
#include <functional>
#include "ConfigFileRW.h"
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
    void TaskFinished(int exitCode);
protected:
    virtual void run()
    {
        QString path(localFilePath_);
        std::wstring u16AppPath = path.toStdWString();
        std::replace(u16AppPath.begin(), u16AppPath.end(), L'/', L'\\');

        auto targetFolder = GetExeFolderW();
        std::wstring param;
        if (targetFolder.size() > 3)
        {
            targetFolder = targetFolder.substr(0, targetFolder.length() - 1);
            auto index = targetFolder.find_last_of(L'\\');
            targetFolder = targetFolder.substr(0, index)+L"\\";
        }
        else {
            qDebug() << u8"不应该被安装到根目录";
        }

        if (ConfigFileReadWriter::Instance().IsSilentInstallationOn())
        {
            //folder = L" /S -PATH=\"" + folder + L"\"";
            param = L" /S";
        }
        if(!ConfigFileReadWriter::Instance().IsDatFileEnabled())
        {
            param+= L" -PATH=\"" + targetFolder + L"\"";
        }
        else
        {
            auto setupFile = GetApplicationDirPath() + "updatepath.dat";

            QFile f(setupFile);
            bool ret = f.open(QIODevice::WriteOnly | QIODevice::Truncate);
            if (ret)
            {
                f.write(QString::fromStdWString(targetFolder).toLocal8Bit());
                f.close();
            }
            else
            {
                param = L" /S -PATH=\"" + targetFolder + L"\"";
            }
        }

        SHELLEXECUTEINFO ShExecInfo = { 0 };
        ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        ShExecInfo.hwnd = NULL;
        ShExecInfo.lpVerb = L"open";
        ShExecInfo.hInstApp = NULL;
        ShExecInfo.lpFile = u16AppPath.c_str();
        ShExecInfo.lpParameters = param.c_str();
        ShExecInfo.nShow = SW_NORMAL;

        ShellExecuteExW(&ShExecInfo);
        if (ShExecInfo.hProcess)
        {
            while (!isInterruptionRequested())
            {
                auto ret = WaitForSingleObject(ShExecInfo.hProcess, 1000);
                if (WAIT_OBJECT_0 == ret)
                {
                    qDebug() << "setup finished ";
                    emit TaskFinished(0);
                    break;
                }
                else if(WAIT_FAILED==ret)
                {
                    qWarning() << "WaitForSingleObject func error, error code is: " << GetLastError();
                    emit TaskFinished(1);
                    break;
                }
                else if (WAIT_TIMEOUT == ret)
                {
                    qDebug() << "time out";
                }
                else if (WAIT_ABANDONED == ret)
                {
                    qWarning() << "setup.exe is corrupted";
                    emit TaskFinished(1);
                    break;
                }
            }
            CloseHandle(ShExecInfo.hProcess);
        }
        else
        {
            qDebug() << "failed to create process";
            emit TaskFinished(2);
        }
    }
private:
    TaskFunc func_;
    QString localFilePath_;
    static std::atomic<bool> hasInstance;
};
