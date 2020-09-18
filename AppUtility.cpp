#include <mutex>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

#include "AppUtility.h"

int g_LogLevel = 0;
HANDLE g_handle = INVALID_HANDLE_VALUE;

//我们不考虑效率，因为这个程序是我第一个qt程序，主要是玩的开心
//这一次我执着面对, 任性的沉醉...
std::mutex g_mutex;

void Logging(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    std::lock_guard lock(g_mutex);

    if (type < g_LogLevel)
        return;

    QString text;
    switch (type)
    {
    case QtDebugMsg:
        text = QString("Debug:");
        break;
    case QtWarningMsg:
        text = QString("Warning:");
        break;
    case QtCriticalMsg:
        text = QString("Critical:");
        break;
    case QtFatalMsg:
        text = QString("Fatal:");
    }

    QString context_info = QString("%1 %2").arg(QString(context.file)).arg(context.line);
    QString current_date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString message = QString("[%1 %2] [%3] %4").arg(text).arg(context_info).arg(current_date).arg(msg);

    QFile file("pkpmUpdate_log.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    //now who's the joke of ....
    if (file.size() > 50 * 1024 * 1024)
        file.resize(0);

    QTextStream text_stream(&file);
    text_stream << message << "\r\n";
    file.flush();
    file.close();
}

void SetLogLevel(int level)
{
    std::lock_guard lock(g_mutex);
    g_LogLevel = level;
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