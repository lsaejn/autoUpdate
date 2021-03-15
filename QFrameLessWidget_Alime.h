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
class PackageListWidget;



class QFrameLessWidget_Alime : public Alime_ContentWidget, noncopyable
{
    Q_OBJECT

public:
    QFrameLessWidget_Alime(QWidget *parent = Q_NULLPTR);

    /// <summary>
    /// Ϊ�����ṩ����
    /// </summary>
    /// <returns>����</returns>
    virtual QString GetTitle() override;

    /// <summary>
    /// ������ʾ
    /// </summary>
    /// <param name=""></param>
    /// <param name="isWaring"></param>
    void SetTips(const QString&, bool isWaring = false);

private:

    /// <summary>
    /// ��ȡpackinfo.json���ݣ���ʼ��ListItem
    /// </summary>
    void ReadPkgFileInfo();

    /// <summary>
    /// ��ȡ��������汾
    /// </summary>
    /// <returns>�Ƿ�ɹ���ȡ</returns>
    bool ReadLocalVersion();
    
    /// <summary>
    /// 
    /// </summary>
    /// <param name=""></param>
    void ShowVersionTipsInfo(const QString&);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="reply"></param>
    void QueryInfoFinish(QNetworkReply* reply);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="pkgFileContent"></param>
    /// <returns></returns>
    bool InitDownloadList(const std::string& pkgFileContent);
 
    /// <summary>
    /// ������������ַ
    /// </summary>
    /// <param name="info">����json</param>
    /// <param name="url">���õ��ĵ�ַ</param>
    /// <param name="showImage">�Ƿ���Ҫ��ʾ����</param>
    void GetUpdatePackUrl(const nlohmann::json& info, std::string& url, bool& showImage);

    bool AddItemToComparisonDownloadWidget(const QString&);

    void ReadFixPacksInfo();//��ǰ�汾�Ĳ�����Ϣ
    void ReadUpdatePacksInfo();//��һ���汾��������Ϣ�Ͳ�����Ϣ
    void ReadInstallationCDInfo();
    void ReadIntegralFilesPackInfo();

    //
    void ReadInstallationCDInfo(PackageListWidget* wgt);
    void ReadFixPacksInfoOfSpecificVersion(PackageListWidget* wgt, const std::string& version);

private:
    QStackedWidget* stackWidget_;
    PackageListWidget* pkgList_;

    QWidget* leftContent_;
    QWidget* rightContent_;

    QLabel* versionTips_;

   
    std::string mainVersionLocal_; //Vxy����ʷԭ�� �����ж�ע����ֵ
    std::string versionLocal_;

    bool netAvailable_;
    nlohmann::json json_;//�����ļ���Ӧ��json
};