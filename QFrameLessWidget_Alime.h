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
    /// 辅助函数
    /// </summary>
    /// <param name="tipInfo">字符串或富文本</param>
    virtual void ShowVersionTipsInfo(const QString& tipInfo);

    /// <summary>
    ///  slot, 处理packinfo.json下载完成信号
    /// </summary>
    /// <param name="reply"></param>
    void QueryInfoFinish(QNetworkReply* reply);

    /// <summary>
    /// 填充QListWidget
    /// </summary>
    /// <param name="pkgFileContent">packinfo.json原始字符</param>
    /// <returns>是否初始化</returns>
    bool InitDownloadList(const std::string& pkgFileContent);

    /// <summary>
    /// 一个封装，内部实现了差异对比。等待移到DownloadInfoWidget
    /// </summary>
    /// <param name="">。。。</param>
    /// <returns></returns>
    bool AddItemToComparisonDownloadWidget(const QString&);

    /// <summary>
    /// 一个封装，内部实现了差异对比。等待移到DownloadInfoWidget
    /// </summary>
    void ReadIntegralFilesPackInfo();

private:

    PackageListWidget* pkgList_;
    QStackedWidget* stackWidget_;
    
    QLabel* versionTips_;
    QWidget* leftContent_;
    QWidget* rightContent_;

    std::string versionLocal_;
    std::string mainVersionLocal_; //Vxy，历史原因 用于判断注册表键值
    
    bool netAvailable_;
    nlohmann::json json_;//packinfo.json
};