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
    /// 为基类提供标题
    /// </summary>
    /// <returns>标题</returns>
    virtual QString GetTitle() override;

    /// <summary>
    /// 设置提示
    /// </summary>
    /// <param name=""></param>
    /// <param name="isWaring"></param>
    void SetTips(const QString&, bool isWaring = false);

private:

    /// <summary>
    /// 读取packinfo.json内容，初始化ListItem
    /// </summary>
    void ReadPkgFileInfo();

    /// <summary>
    /// 读取本地软件版本
    /// </summary>
    /// <returns>是否成功读取</returns>
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
    /// 返回升级包地址
    /// </summary>
    /// <param name="info">完整json</param>
    /// <param name="url">欲得到的地址</param>
    /// <param name="showImage">是否需要显示光盘</param>
    void GetUpdatePackUrl(const nlohmann::json& info, std::string& url, bool& showImage);

    bool AddItemToComparisonDownloadWidget(const QString&);

    void ReadFixPacksInfo();//当前版本的补丁信息
    void ReadUpdatePacksInfo();//下一个版本的升级信息和补丁信息
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

   
    std::string mainVersionLocal_; //Vxy，历史原因 用于判断注册表键值
    std::string versionLocal_;

    bool netAvailable_;
    nlohmann::json json_;//网络文件对应的json
};