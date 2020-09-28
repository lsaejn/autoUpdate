#include <mutex>

#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QApplication>
#include <QMessageBox>
#include <QProcess>

#include "AppUtility.h"
#include "ConfigFileRW.h"

HANDLE g_handle = INVALID_HANDLE_VALUE;
std::mutex g_mutex;

//不考虑效率
void Logging(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    std::lock_guard<std::mutex> lock(g_mutex);

    if (static_cast<size_t>(type) < ConfigFileReadWriter::Instance().GetLogLevel())
        return;

    QString text;
    switch (type)
    {
    case QtDebugMsg://开发
        text = QString("Debug:");
        break;
    case QtWarningMsg://测试
        text = QString("Warning:");
        break;
    case QtCriticalMsg://用户
        text = QString("Critical:");
        break;
    case QtFatalMsg:
        text = QString("Fatal:");
        break;
    }
    QString context_info = QString("%1 %2").arg(QString(context.file)).arg(context.line);
    QString current_date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString message = QString("[%1 %2] [%3] %4").arg(text).arg(context_info).arg(current_date).arg(msg);

    //低效
    QFile file(ConfigFileReadWriter::Instance().GetLogFilePath());
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    if (file.size() > 50 * 1024 * 1024)
        file.resize(0);

    QTextStream text_stream(&file);
    text_stream << message << "\r\n";
    file.flush();
    file.close();
}

size_t ToLogLevel(const std::string& level)
{
    if (level == "Debug")
        return 0;
    else if (level == "Warning")
        return 1;
    else if (level == "Critical")
        return 2;
    else if (level == "None")//
        return 3;
    return 0;
}

bool IsInstanceOn()
{
    g_handle=::CreateEventW(NULL, FALSE, TRUE, L"updateExe.cn.pkpm.www");
    if (g_handle != INVALID_HANDLE_VALUE && ERROR_ALREADY_EXISTS == GetLastError())
    {
        CloseHandle(g_handle);
        return false;
    }
    return true;
}

std::string GetExeFolder()
{
    char buffer[1024*8];
    GetModuleFileNameA(NULL, buffer, sizeof(buffer) / sizeof(*buffer));
    int pos = -1;
    int index = 0;
    while (buffer[index])
    {
        if (buffer[index] == '\\' || buffer[index] == '/')
        {
            pos = index;
        }
        index++;
    }
    return std::string(buffer, pos + 1);
}

std::wstring GetExeFolderW()
{
    wchar_t buffer[1024 * 8];
    GetModuleFileName(NULL, buffer, sizeof(buffer) / sizeof(*buffer));
    int pos = -1;
    int index = 0;
    while (buffer[index])
    {
        if (buffer[index] == '\\' || buffer[index] == '/')
        {
            pos = index;
        }
        index++;
    }
    return std::wstring(buffer, pos + 1);
}

QString GetStyleName()
{
    auto path=GetExeFolderW() + L"..\\CFG\\PKPM.ini";
    int index=GetPrivateProfileIntW(L"InterfaceStyle", L"index", 1, path.c_str());
    index = 0;
    if (index == 0)
        return ":/qss/dark.qss";
    return ":/qss/blue.qss";
}

/*
exe会被放在update文件夹. 下载的文件被放在update/download/下
*/
QString GetDownloadFolder()
{
    QString appDirectory = ConfigFileReadWriter::Instance().GetDownloadFolder();
    bool ret = !appDirectory.isEmpty() || !QFileInfo(appDirectory).isDir();
    if (!ret)
    {
        qCritical() << "error happened in GetDownloadFolder due to limited read access priviledge?";
        std::abort();
    }
    else
    {
        return appDirectory;
    }
}

QString GetFolderPart(const QString& path)
{
    QString result;
    size_t index = 0;
    for (size_t i = 0; i != path.size(); ++i)
    {
        if (path[i] == '/' || path[i] == '\\')
            index = i;
    }
    return path.mid(0, index + 1);
}

double ToMByte(int sizeInBit)
{
    return sizeInBit * 1.0 / (1024 * 1024);
}

double ToKByte(int sizeInBit)
{
    return sizeInBit * 1.0 / (1024);
}

void ShowWarningBox(const QString& title, const QString& waring, const QString& btnText)
{
    QMessageBox warningBox(QMessageBox::Warning, title, waring);
    warningBox.setStandardButtons(QMessageBox::Ok);
    warningBox.setButtonText(QMessageBox::Ok, btnText);
    warningBox.exec();
}

bool ShowQuestionBox(const QString& title, const QString& info, const QString& yesText, const QString& noText)
{
    QMessageBox infoBox(QMessageBox::Question, title, info);
    infoBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No);
    infoBox.setButtonText(QMessageBox::Yes, yesText);
    infoBox.setButtonText(QMessageBox::No, noText);
    return infoBox.exec()== QMessageBox::Yes;
}

void CreateFolderForApp()
{
    QString downloadDir = ConfigFileReadWriter::Instance().GetDownloadFolder();
    QDir dirInfo;
    if (!dirInfo.exists(downloadDir))
    {
        bool ret=dirInfo.mkdir(downloadDir);
        if (!ret)
        {
            auto filePath = downloadDir.mid(0, downloadDir.length() - 1);
            QFileInfo f(filePath);
            if (f.exists() && f.isFile())
            {
                if (!QFile::remove(filePath))
                {
                    //what the fuck?
                }
            }
            dirInfo.mkdir(downloadDir);
        }
    }      
    QString logDir = downloadDir + "../log/";
    if (!dirInfo.exists(logDir))
        dirInfo.mkdir(logDir);
}

void OpenLocalPath(const QString& path)
{
    QString filePath = path;
    QProcess process;
    QString cmd;
    if (!QFile::exists(path))
    {
        filePath = GetFolderPart(path);
        filePath.replace("/", "\\"); // 只能识别 "\"
        cmd = QString("explorer.exe /open,\"%1\"").arg(filePath);
    }
    else
    {
        filePath.replace("/", "\\");
        cmd = QString("explorer.exe /select,\"%1\"").arg(filePath);
    }
    process.startDetached(cmd);
}