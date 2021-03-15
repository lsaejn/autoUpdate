#pragma once

#include <QtWidgets/QWidget>

#include "thirdParty/nlohmann/json.hpp"

#include "Alime_ContentWidget.h"
#include "Alime/noncopyable.h"

class QLabel;
class QListWidget;
class QNetworkReply;
class QStackedWidget;
class PackageListWidget;
class DownloadInfoWidget;


class QFrameLessWidget_Alime :
    public Alime_ContentWidget, noncopyable
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
    /// ��������
    /// </summary>
    /// <param name="tipInfo">�ַ������ı�</param>
    virtual void ShowVersionTipsInfo(const QString& tipInfo);

    /// <summary>
    ///  slot, ����packinfo.json��������ź�
    /// </summary>
    /// <param name="reply"></param>
    void QueryInfoFinish(QNetworkReply* reply);

    /// <summary>
    /// ���QListWidget
    /// </summary>
    /// <param name="pkgFileContent">packinfo.jsonԭʼ�ַ�</param>
    /// <returns>�Ƿ��ʼ��</returns>
    bool InitDownloadList(const std::string& pkgFileContent);

    /// <summary>
    /// һ����װ���ڲ�ʵ���˲���Աȡ��ȴ��Ƶ�DownloadInfoWidget
    /// </summary>
    /// <param name="">������</param>
    /// <returns></returns>
    bool AddItemToComparisonDownloadWidget(const QString&);

    /// <summary>
    /// һ����װ���ڲ�ʵ���˲���Աȡ��ȴ��Ƶ�DownloadInfoWidget
    /// </summary>
    void ReadIntegralFilesPackInfo();

private:

    PackageListWidget* pkgList_;
    QStackedWidget* stackWidget_;
    
    QLabel* versionTips_;
    QWidget* leftContent_;
    QWidget* rightContent_;

    std::string versionLocal_;
    std::string mainVersionLocal_; //Vxy����ʷԭ�� �����ж�ע����ֵ
    
    bool netAvailable_;
    nlohmann::json json_;//packinfo.json
};