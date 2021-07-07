#pragma once
#include <QListWidget>
#include <atomic>

#include "thirdParty/nlohmann/json.hpp"
#include "TypeDefine.h"
//fix me

/*
*需求突然修改导致以下问题:
*1.业务上不能再同时更新两个StackWidget里的item了，否则文件会错乱。
* 因此，当一键更新时，我禁止了切换StackWidget的切换，
* 又因此，一个更新按钮就可以了。StackWidget只塞SetupWidget
*2.一键更新被打断的方式太多了
* 无论如何，SetupWidget里的自定义widget和更新按钮都不能做绑定，
* 因为用户随时可以操作按钮。
* 因此，当一键更新运行时，我直接禁止了用户操作以便视这次更新为一次完整事务。
* 这样，用最简单的方式解决了交互问题。
*/
//而SetupWidget和更新按钮是同级的，这导致了一些交互问题
//现在SetupWidget
///好吧，到了三月份，PackageListWidget包含升级包/光盘, 补丁包
///要在一个页面显示。
///光盘作为升级包的一种

class DownloadInfoWidget;
class QFrameLessWidget_Alime;


class PackageListWidget : public QListWidget
{
    Q_OBJECT
signals:
    void installing(int);
    void finish(int);
    void error();
    void reset();

public:
    PackageListWidget(QWidget* parent);

    DownloadInfoWidget* AddItem(QWidget* _parent,qint64 _fileSize, const QUrl& _url, const QUrl& ins_url, const QString& filename,
        PackType ty);

    void Parse(const nlohmann::json& json_);
    void GetUpdatePackUrl(const nlohmann::json& json, std::string& urlOut, std::string& instructionUrl, bool& showImage);
    void ReadUpdatePack(const nlohmann::json& json_);
    void ReadFixPack(const nlohmann::json& json_);
    void ReadUpdateFolder(const nlohmann::json& json_);

    //本widget下是否有item正在下载/安装
    //不允许补丁包和升级包同时进行
    bool HasSetupItem();

    void SetVersion(const std::string& mainV, const std::string& localV);

    void SetMainWidget(QFrameLessWidget_Alime* p) { mainWidget_ = p; };
private:
    void ReReadPacks();
    void ReadSetupImage(const nlohmann::json& json_);

    std::string mainVersionLocal_; //Vx.y，历史原因 用于判断注册表键值
    std::string versionLocal_;
    QFrameLessWidget_Alime* mainWidget_;
};
