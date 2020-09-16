#pragma once

#include "VersionFileFinder.h"
#include <QString>
#include <QApplication>

//����һ������ "5.1.1" �Ľ��
QString GetLocalVersion()
{
    auto versionFiles = FindSpecificFiles::FindVersionFiles(QApplication::applicationDirPath().toLocal8Bit().data(), "V", "ini");
    //�����ļ���fuck��
    if (versionFiles.size() == 0)
    {
        return "";
    }
    std::sort(versionFiles.begin(), versionFiles.end(), AscendingOrder());
    return versionFiles.back().c_str();
}
