#pragma once

#include <QtWidgets/QWidget>
#include "Alime_ContentWidget.h"

class DownloadInfoWidget;
class QListWidget;
class QNetworkReply;

class QFrameLessWidget_Alime : public Alime_ContentWidget
{
    Q_OBJECT

public:
    QFrameLessWidget_Alime(QWidget *parent = Q_NULLPTR);
    virtual QString GetTitle() override;

private:
    bool InitDownloadList(const std::string& pkgFileContent);
    void ReadPkgFileInfo();
    void QueryInfoFinish(QNetworkReply* reply);
    QListWidget* downloadList_;
    QWidget* leftContent_;
    QWidget* rightContent_;
};