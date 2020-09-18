#pragma once

#include <QString>
#include <QApplication>

#include "VersionFileFinder.h"
#include "AppUtility.h"

//����һ������ "5.1.1" �Ľ��

class LocalVersionFile
{
public:
    LocalVersionFile()
    {
        folder_= GetExeFolder()+"CFG";
    }

    ~LocalVersionFile() = default;

    void SetVersionFileFolder(const std::string& folder)
    {
        folder_ = folder;
    }

    std::string GetLocalVersion()
    {
        auto versionFiles = FindSpecificFiles::FindVersionFiles(folder_.c_str(), "V", "ini");
        if (versionFiles.size() == 0)
        {
            return "";
        }
        std::sort(versionFiles.begin(), versionFiles.end(), AscendingOrder());
        return versionFiles.back().c_str();
    }

private:
    std::string folder_;
};
