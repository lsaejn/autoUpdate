#pragma once

#include <QtWidgets/QWidget>

#include "thirdParty/nlohmann/json.hpp"
#include "Alime_ContentWidget.h"

#include "Alime/noncopyable.h"

class DownloadInfoWidget;
class QListWidget;
class QNetworkReply;
class QLabel;
class QStackedWidget;
class SetupWidget;



class QFrameLessWidget_Alime : public Alime_ContentWidget, noncopyable
{
    Q_OBJECT

public:
    QFrameLessWidget_Alime(QWidget *parent = Q_NULLPTR);

    virtual QString GetTitle() override;

    void SetTips(const QString&, bool isWaring = false);
private:
    void ReadPkgFileInfo();

    /// <summary>
    /// ��ȡ��������汾
    /// </summary>
    /// <returns>�Ƿ�ɹ���ȡ</returns>
    bool ReadLocalVersion();
    
    void ShowVersionTipsInfo(const QString&);
    void QueryInfoFinish(QNetworkReply* reply);
    bool InitDownloadList(const std::string& pkgFileContent);
 
    std::vector<std::string> GetFilteredVersionKeys(const nlohmann::json& info);

    //static
    DownloadInfoWidget* AddNewItemAndWidgetToList(QListWidget* target, QWidget* _parent,
        qint64 _fileSize, const QUrl& _url, const QString& filename);

    bool AddItemToComparisonDownloadWidget(const QString&);

    void ReadFixPacksInfo();//��ǰ�汾�Ĳ�����Ϣ
    void ReadUpdatePacksInfo();//��һ���汾��������Ϣ�Ͳ�����Ϣ
    void ReadInstallationCDInfo();
    void ReadIntegralFilesPackInfo();

    //
    void ReadInstallationCDInfo(SetupWidget* wgt);
    //void ReadFixPacksInfo(SetupWidget* wgt);
    void ReadFixPacksInfoOfSpecificVersion(SetupWidget* wgt, const std::string& version);
private:
    QStackedWidget* stackWidget_;
    SetupWidget* updatePkgList_;
    SetupWidget* fixPkgList_;
    SetupWidget* imageWidget_;
    QListWidget* integralFilesPackList_;

    QWidget* leftContent_;
    QWidget* rightContent_;
    QLabel* versionTips_;
    std::string mainVersionLocal_;
    std::string versionLocal_;

    bool netAvailable_;
    nlohmann::json json_;//�����ļ���Ӧ��json
};