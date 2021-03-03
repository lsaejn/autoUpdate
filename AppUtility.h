#pragma once

#include <QString>
#include <QDebug>
#include <QUrl>
#include "windows.h"

extern HANDLE g_handle;
extern HWND g_hwnd;

bool IsMutexNew();
size_t ToLogLevel(const std::string& level);

std::string GetExeFolder();
std::wstring GetExeFolderW();
std::wstring GetParentFolderW(const std::wstring&);

QString GetApplicationDirPath();

QString GetPkpmRootPath();

QString GetStyleName();
QString GetStyleFilePath();

QString GetDownloadFolder();
QString GetFolderPart(const QString&);
QString GetFilePart(const QUrl& url);
QString GetFilePart(const QString&);
void Logging(QtMsgType type, const QMessageLogContext& context, const QString& msg);

void OpenLocalPath(const QString&);

double ToMByte(long long sizeInBit);
double ToKByte(long long sizeInBit);
QString MakeDownloadSpeed(long long bytesInSecond);

void ShowWarningBox(const QString& title, const QString& waring, const QString& btnText);
bool ShowQuestionBox(const QString& title, const QString& info, const QString& yesText, const QString& noText);
void CreateFolderForApp();

bool IsSameRegKey(const std::string& version1, const std::string& version2);

bool UnzipFile(const QString zipFilePath, const QString& TargetPath);


#define CHECK_CONNECT_ERROR(expr) \
if(!expr) \
qDebug()<<"connect failed"
