#pragma once

#include <QString>
#include <QDebug>
#include "windows.h"

extern HANDLE g_handle;

bool IsInstanceOn();
size_t ToLogLevel(const std::string& level);

std::string GetExeFolder();
std::wstring GetExeFolderW();
QString GetStyleName();

QString GetDownloadFolder();
QString GetFolderPart(const QString&);
void Logging(QtMsgType type, const QMessageLogContext& context, const QString& msg);

void OpenLocalPath(const QString&);

double ToMByte(int sizeInBit);
double ToKByte(int sizeInBit);

void ShowWarningBox(const QString& title, const QString& waring, const QString& btnText);
bool ShowQuestionBox(const QString& title, const QString& info, const QString& yesText, const QString& noText);
void CreateFolderForApp();

#define CHECK_CONNECT_ERROR(expr) \
if(!expr) \
qDebug()<<"connect failed"
