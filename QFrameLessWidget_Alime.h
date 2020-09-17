#pragma once

#include <QtWidgets/QWidget>
#include "Alime_ContentWidget.h"

class DownloadInfoWidget;
class QListWidget;
class QNetworkReply;
class QLabel;

/*
�������Ҫˮƽ�ָ�ı�����
�̳�Alime_ContentWidget�����Ϳ�����
*/

class QFrameLessWidget_Alime : public Alime_ContentWidget
{
    Q_OBJECT

public:
    QFrameLessWidget_Alime(QWidget *parent = Q_NULLPTR);
    virtual QString GetTitle() override;

private:
    void ReadPkgFileInfo();
    void ReadInstallationCDInfo();
    void ShowVersionTipsInfo(const QString&);
    void QueryInfoFinish(QNetworkReply* reply);
    bool InitDownloadList(const std::string& pkgFileContent);

private:
    QListWidget* downloadList_;
    QListWidget* isoFileList;
    QWidget* leftContent_;
    QWidget* rightContent_;
    QLabel* versionTips_;
};