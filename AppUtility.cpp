#include <mutex>

#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QApplication>
#include <QMessageBox>
#include <QProcess>

#include "AppUtility.h"
#include "String_utility.h"
#include "ConfigFileRW.h"

HANDLE g_handle = INVALID_HANDLE_VALUE;
HWND g_hwnd=0;
std::mutex g_mutex;

//不考虑效率
void Logging(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    std::lock_guard<std::mutex> lock(g_mutex);

    if (static_cast<size_t>(type) < ConfigFileReadWriter::Instance().GetLogLevel())
        return;

    QString level;//Qt的loglevel有点古怪
    switch (type)
    {
    case QtDebugMsg://开发
        level = QString("Debug");
        break;
    case QtWarningMsg://测试
        level = QString("Warning");
        break;
    case QtCriticalMsg://用户
        level = QString("Critical");
        break;
    case QtFatalMsg:
        level = QString("Fatal");
        break;
    }
    QString qs = context.file;
    auto index=qs.lastIndexOf("\\");
    QString fileName = qs.mid(index + 1);
    QString context_info = QString("%1-%2").arg(QString(fileName)).arg(context.line);
    QString current_date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString message = QString("[%1] [%2] [%3] %4").arg(level).arg(context_info).arg(current_date).arg(msg);

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

bool IsMutexNew()
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

std::wstring GetParentFolderW(const std::wstring& path_)
{
    auto path = path_;
    if (path.back() == L'\\' || path.back() == L'/')
    {
        path = path.substr(0, path.size() - 1);
    }
    auto index = path.find_last_of(L"/\\");
    if (index != std::wstring::npos)
    {
        return path.substr(0, index)+L"\\";
    }
    return {};
}

QString GetApplicationDirPath()
{
    return QCoreApplication::applicationDirPath() + "/";
}

QString GetPkpmRootPath()
{
    auto installerPath = QCoreApplication::applicationDirPath() + "/";
    auto rootPath = installerPath + "../";
    QDir dir(rootPath);
    QString s=dir.absolutePath();
    return s+"/";
}

QString GetStyleFilePath()
{
    return GetApplicationDirPath() + "..\\CFG\\PKPM.ini";
}

QString GetStyleName()
{
    auto path=GetExeFolderW() + L"..\\CFG\\PKPM.ini";
    int index=GetPrivateProfileIntW(L"InterfaceStyle", L"index", -1, path.c_str());
    if (index != 1)//0 or file not found
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
    size_t index = 0;
    for (int i = 0; i != path.size(); ++i)
    {
        if (path[i] == '/' || path[i] == '\\')
            index = i;
    }
    return path.mid(0, index + 1);
}

QString GetFilePart(const QString& path)
{
    auto index = path.lastIndexOf('/');
    return path.mid(index + 1);
}

QString GetFilePart(const QUrl& qUrl)
{
    auto fullName = qUrl.toString();
    return GetFilePart(fullName);
}


double ToMByte(long long sizeInBit)
{
    return sizeInBit * 1.0 / (1024 * 1024);
}

double ToKByte(long long sizeInBit)
{
    return sizeInBit * 1.0 / (1024);
}

QString MakeDownloadSpeed(long long bytesInSecond)
{
    if (bytesInSecond < 1024)
    {
        QString result = QString::number(bytesInSecond);
        result += " B/s";
        return result;
    }
    double kBytesInSecond = bytesInSecond / 1024;
    if (kBytesInSecond < 1024)
    {
        QString result = QString::number(kBytesInSecond);
        result += " KB/s";
        return result;
    }
    else
    {
        double mBytes=kBytesInSecond /1024;
        QString result = QString::number(mBytes, 'g', 2);
        result += " MB/s";
        return result;
    }
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

    QFileInfo fileinfo(filePath);
    if (fileinfo.isDir())
    {
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


bool IsSameRegKey(const std::string& v1, const std::string& v2)
{

    if (v1.empty() || v2.empty())
        return false;
    //for reason that old file s name is V4
    auto version1 = v1 + ".0";
    auto version2 = v2 + ".0";
    bool v1StartsWithV = version1.front() == 'V';
    bool v2StartsWithV = version2.front() == 'V';
    // escape 'V'
    if(v1StartsWithV&& v2StartsWithV)
        return string_utility::startsWith(version1.c_str(), version2.c_str(), 4);
    return string_utility::startsWith(version1.c_str(), version2.c_str(), 3);
}

//we copy this file from github-netease/nim-duilibframework
#include "thirdParty/zlib/UnZip.h"

bool UnzipFile(const QString zipFilePath, const QString& TargetPath)
{
    return true;
}

