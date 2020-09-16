#pragma once

#include "VersionFileFinder.h"
#include <QString>
#include <QApplication>

//返回一个类似 "5.1.1" 的结果
QString GetLocalVersion()
{
    auto versionFiles = FindSpecificFiles::FindVersionFiles(QApplication::applicationDirPath().toLocal8Bit().data(), "V", "ini");
    //本地文件被fuck了
    if (versionFiles.size() == 0)
    {
        return "";
    }
    std::sort(versionFiles.begin(), versionFiles.end(), AscendingOrder());
    return versionFiles.back().c_str();
}
