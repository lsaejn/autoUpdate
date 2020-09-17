#pragma once

#include <QString>
#include "windows.h"

extern int g_LogLevel;
extern HANDLE g_handle;

void Logging(QtMsgType type, const QMessageLogContext& context, const QString& msg);

void SetLogLevel(int);

bool IsInstanceOn();

