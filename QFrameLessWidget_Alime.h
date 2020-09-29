#pragma once

#include <QtWidgets/QWidget>
#include "thirdParty/nlohmann/json.hpp"
#include "Alime_ContentWidget.h"

class DownloadInfoWidget;
class QListWidget;
class QNetworkReply;
class QLabel;
class QStackedWidget;
class SetupImageWidget;

/*
如果不需要水平分割的标题栏
继承Alime_ContentWidget这个类就可以了
*/

class QFrameLessWidget_Alime : public Alime_ContentWidget
{
    Q_OBJECT

public:
    QFrameLessWidget_Alime(QWidget *parent = Q_NULLPTR);
    virtual QString GetTitle() override;
    void SetTips(const QString&, bool isWaring = false);
private:
    void ReadPkgFileInfo();
    bool ReadLocalVersion();
    
    void ShowVersionTipsInfo(const QString&);
    void QueryInfoFinish(QNetworkReply* reply);
    bool InitDownloadList(const std::string& pkgFileContent);

    //QWidget* MakeNewListItem();
    
    std::vector<std::string> GetFilteredVersionKeys(const nlohmann::json& info);

    bool AddNewItemAndWidgetToList(QListWidget* target, QWidget* _parent, 
        qint64 _fileSize, const QString& _url);

    void ReadFixPacksInfo();
    void ReadUpdatePacksInfo();
    void ReadInstallationCDInfo();

private:
    QStackedWidget* stackWidget_;
    QListWidget* updatePkgList_;
    QListWidget* isoFileList_;
    QListWidget* fixPkgList_;
    SetupImageWidget* imageWidget_;

    QWidget* leftContent_;
    QWidget* rightContent_;
    QLabel* versionTips_;
    std::string mainVersionLocal_;
    std::string versionLocal_;

    bool netAvailable_;
    nlohmann::json json_;//网络文件对应的json
};