#pragma once

#include <QtWidgets/QWidget>
#include "thirdParty/nlohmann/json.hpp"
#include "Alime_ContentWidget.h"

class DownloadInfoWidget;
class QListWidget;
class QNetworkReply;
class QLabel;
class QStackedWidget;

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
    
    void ShowVersionTipsInfo(const QString&);
    void QueryInfoFinish(QNetworkReply* reply);
    bool InitDownloadList(const std::string& pkgFileContent);

    bool ReadLocalVersion();
    //QWidget* MakeNewListItem();
    
    std::vector<std::string> GetFilteredVersionKeys(const nlohmann::json& info);

    bool AddNewItemAndWidgetToList(QListWidget* target, QWidget* _parent, 
        uint64_t _fileSize, const QString& _url);

    void ReadFixPacksInfo();
    void ReadUpdatePacksInfo();
    void ReadInstallationCDInfo(const nlohmann::json& info);
    

private:
    QStackedWidget* stackWidget_;
    QListWidget* updatePkgList_;
    QListWidget* isoFileList_;
    QListWidget* fixPkgList_;

    QWidget* leftContent_;
    QWidget* rightContent_;
    QLabel* versionTips_;
    std::string mainVersionLocal_;
    std::string versionLocal_;

    nlohmann::json json_;//�����ļ���Ӧ��json
};