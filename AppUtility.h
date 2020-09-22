#pragma once

#include <QString>
#include <QDebug>
#include "windows.h"

extern int g_LogLevel;
extern HANDLE g_handle;

bool IsInstanceOn();
void SetLogLevel(int);
std::string GetExeFolder();
QString GetDownloadFolder();
void Logging(QtMsgType type, const QMessageLogContext& context, const QString& msg);
double ToMByte(int sizeInBit);
