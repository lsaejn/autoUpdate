#include <QLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QMessageBox>
#include <QUrl>
#include <QApplication>
#include <QStackedWidget>
#include <QButtonGroup>

#include "QtNetwork/qnetworkaccessmanager.h"
#include "QtNetwork/qnetworkrequest.h"
#include "QtNetwork/qnetworkreply.h"

#include "DownloadInfoWidget.h"
#include "QFrameLessWidget_Alime.h"
#include "VersionFileFinder.h"
#include "AppVersion.h"
#include "ConfigFileRW.h"
#include "SetupImageWidget.h"

CLASSREGISTER(QFrameLessWidget_Alime)

QFrameLessWidget_Alime::QFrameLessWidget_Alime(QWidget* parent)
    : Alime_ContentWidget(parent),
    netAvailable_(false),
    stackWidget_(nullptr),
    updatePkgList_(nullptr),
    isoFileList_(nullptr),
    fixPkgList_(nullptr),
    imageWidget_(nullptr),
    leftContent_(nullptr),
    rightContent_(nullptr),
    versionTips_(nullptr)
{
    {
        ReadPkgFileInfo();
        ReadLocalVersion();
    }
    QHBoxLayout* contentLayout = new QHBoxLayout(this);
    leftContent_ = new QWidget(this);
    rightContent_ = new QWidget(this);
    contentLayout->addWidget(leftContent_);
    contentLayout->addWidget(rightContent_);
    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);
    contentLayout->setStretch(0, 1);
    contentLayout->setStretch(1, 4);

    leftContent_->setObjectName("leftContent");

    QVBoxLayout* leftLayout = new QVBoxLayout(leftContent_);
    leftLayout->setMargin(0); 
    
    QWidget* leftButtons = new QWidget(this);//按钮在左上
    leftLayout->addWidget(leftButtons);
    QWidget* dummy = new QWidget(this);//下面放一个占位
    leftLayout->addWidget(dummy);
    leftLayout->setStretch(0, 2);
    leftLayout->setStretch(1, 3);

    ///add left
    ///add left button
    QVBoxLayout* leftButtonLayout = new QVBoxLayout(leftButtons);
    leftButtonLayout->setMargin(0);
    leftButtonLayout->setSpacing(0);
    leftButtonLayout->addSpacing(20);

    QPushButton* btn01 = new QPushButton(u8"升级包");
    btn01->setObjectName("btnBoard");
    btn01->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btn01->setCheckable(true);
    btn01->setChecked(true);
    leftButtonLayout->addWidget(btn01);

    QPushButton* btn02 = new QPushButton(u8"补丁包");
    btn02->setObjectName("btnBoard");
    btn02->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btn02->setCheckable(true);
    leftButtonLayout->addWidget(btn02);

    QPushButton* btn03 = new QPushButton(u8"最新版光盘");
    btn03->setObjectName("btnBoard");
    btn03->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btn03->setCheckable(true);
    leftButtonLayout->addWidget(btn03);

    //QPushButton* btn04 = new QPushButton(u8"差异更新/测试");
    //btn04->setObjectName("btnBoard");
    //btn04->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //btn04->setCheckable(true);
    //leftButtonLayout->addWidget(btn04);

    QButtonGroup* group = new QButtonGroup(this);
    group->addButton(btn01, 0);
    group->addButton(btn02, 1);
    group->addButton(btn03, 2);

    //group->addButton(btn04, 3);


    
    //leftContent->setFixedWidth(240);

    //整个右边就是vbox，加入布局是为了保持一定的灵活性，方便以后加入其他内容(竖条)。
    rightContent_->setObjectName("rightContent");
    QVBoxLayout* vbox = new QVBoxLayout(rightContent_);

    versionTips_ = new QLabel();
    versionTips_->setObjectName("versionTips");

    if (!versionLocal_.empty())
    {
        ////fix me
        versionTips_->setText((u8"检查到当前版本:" + versionLocal_ + "   " + u8"找到以下可升级版本").c_str());
    }
    else
    {
        QString strHTML = QString("<html><head><style> #f{font-size:18px; color: red;} /style></head>\
                            <body><font id=\"f\">%1</font></body></html>").arg(u8"无法查询本地版本信息, 请从官网重新下载完整程序");
        ShowVersionTipsInfo(strHTML);
    }

    vbox->addWidget(versionTips_);

    updatePkgList_ = new QListWidget(this);
    updatePkgList_->setObjectName("updatePkgList");
    fixPkgList_ = new QListWidget(this);
    fixPkgList_->setObjectName("fixPkgList");
    imageWidget_ = new SetupImageWidget(this);

    stackWidget_ = new QStackedWidget(this);
    stackWidget_->addWidget(updatePkgList_);
    stackWidget_->addWidget(fixPkgList_);
    stackWidget_->addWidget(imageWidget_);

    //不使用ID是担心可能要调整顺序 0.0
    connect(group, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
        stackWidget_, &QStackedWidget::setCurrentIndex);

    //fix me, 删掉这个连接。
    //每次更新完成后，重新读版本信息文件，然后刷新列表
    connect(group, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
        [=]() {
            LocalVersionFile finder;
            auto newVersion=finder.GetLocalVersion();
            if (newVersion.empty())
                return;
            QString version = QString("V")+ newVersion.c_str();
            //versionTips_->setText(u8"检查到当前版本:" + version);
            if (netAvailable_)
            {
                QString tips= version+ u8"   找到以下可用信息";
                versionTips_->setText(u8"检查到当前版本:" + tips);
            }
            else
                return;
            //fix me, 没有测试过这一块
            if (version != versionLocal_.c_str())
            {
                updatePkgList_->clear();
                fixPkgList_->clear();
                //imageWidget_->clear();
                ReadLocalVersion();
                ReadUpdatePacksInfo();//fix me,去掉参数
                ReadFixPacksInfo();//fix me
                //ReadInstallationCDInfo();
            }
        });

    vbox->addWidget(stackWidget_);
}

void QFrameLessWidget_Alime::ReadPkgFileInfo()
{
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &QFrameLessWidget_Alime::QueryInfoFinish);
    //qDebug() << manager->supportedSchemes();
    manager->get(QNetworkRequest(QUrl(ConfigFileReadWriter::Instance().GetUrlOfUpdateInfoFile())));
}

void QFrameLessWidget_Alime::QueryInfoFinish(QNetworkReply* reply)
{
    auto ret=reply->error();
    if (ret != QNetworkReply::NoError)
    {
        qWarning() << "Error Happened";
        SetTips(u8"无法检查到程序升级信息，请检查网络", true);
        netAvailable_ = false;
    }
    else
    {
        netAvailable_ = true;
        const QByteArray reply_data = reply->readAll();
        QString str(reply_data);
        InitDownloadList(str.toStdString());
    }
}

QString QFrameLessWidget_Alime::GetTitle()
{
    return u8"Pkpm升级程序";
}

bool QFrameLessWidget_Alime::ReadLocalVersion()
{
    auto versionFiles = FindSpecificFiles::FindVersionFiles(
        (QApplication::applicationDirPath()+u8"/../CFG/").toLocal8Bit().data(), "V", "ini");
    if (versionFiles.size() == 0)
    {
        qWarning() << u8"无法找到版本信息文件";
        return false;
    }
    else
    {
        std::sort(versionFiles.begin(), versionFiles.end(), AscendingOrder());
        versionLocal_ = "V" + versionFiles.back();
        mainVersionLocal_ = "V";
        mainVersionLocal_.push_back(versionFiles.back().front());
    }
    return true;
}

//这个函数不允许出错
bool QFrameLessWidget_Alime::InitDownloadList(const std::string& str)
{
    if (!ReadLocalVersion())
    {
        //fix me, show info "无法读取本地版本信息"
        return false;
    }
    try
    {
        json_ = nlohmann::json::parse(str);
        ReadUpdatePacksInfo();
        ReadFixPacksInfo();
        ReadInstallationCDInfo();
    }
    catch (...)
    {
        qWarning() << "can not parse info from webInfo.json";
    }
    return true;
}

void QFrameLessWidget_Alime::ReadInstallationCDInfo()
{
    auto dubugString = json_.dump();
    std::string isoFileUrl= json_["LatestIsoUrl"];
    //使用qt https需要额外的库, 对于维护xp机器的人很痛苦
    if (string_utility::startsWith(isoFileUrl.c_str(), "https"))
    {
        isoFileUrl.erase(4, 1);
    }
    QNetworkAccessManager manager;
    QString url(isoFileUrl.c_str());
    QEventLoop loop;
    QNetworkReply* reply = manager.head(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()), Qt::DirectConnection);
    loop.exec();
    QVariant var = reply->header(QNetworkRequest::ContentLengthHeader);
    if (reply->error())
    {
        qDebug() << u8"查询光盘信息时出错";
        qDebug() << reply->errorString();
        reply->deleteLater();
        return;
    }
    reply->deleteLater();

    qint64 pkgSize = var.toLongLong();
    AddNewItemAndWidgetToList(imageWidget_, this, pkgSize, url);
}

bool QFrameLessWidget_Alime::AddNewItemAndWidgetToList(QListWidget* target, QWidget* /*_parent*/,
    qint64 _fileSize, const QString& _url)
{
    QListWidgetItem* item = new QListWidgetItem();
    QSize preferSize = item->sizeHint();
    item->setSizeHint(QSize(preferSize.width(), 70));
    target->addItem(item);
    auto index = _url.lastIndexOf("/");
    auto itemWidget = new DownloadInfoWidget(this, _url.mid(index + 1), _fileSize, _url);
    target->setItemWidget(item, itemWidget);
    return true;
}

//copy了代码，带来不好的味道。
//需要测试
void QFrameLessWidget_Alime::ReadFixPacksInfo()
{
    const nlohmann::json& json = json_["FixPacks"];

    if (!versionLocal_.empty() && json.find(versionLocal_) != json.end())
    {
        nlohmann::json array = json_["FixPacks"][versionLocal_];
        QNetworkAccessManager manager;//网上的意思是最多5个请求
        for (size_t i = 0; i != array.size(); ++i)
        {
            auto str=array[i].get<std::string>();
            QString url = ConfigFileReadWriter::Instance().GetUrlOfFixPackFolder() + str.c_str();
            QEventLoop loop;
            QNetworkReply* reply = manager.head(QNetworkRequest(url));
            connect(reply, SIGNAL(finished()), &loop, SLOT(quit()), Qt::DirectConnection);
            loop.exec();

            QVariant var = reply->header(QNetworkRequest::ContentLengthHeader);
            if (reply->error())
            {
                qDebug() << reply->errorString();
                reply->deleteLater();
                continue;
            }
            reply->deleteLater();

            int pkgSize = var.toInt();
            AddNewItemAndWidgetToList(fixPkgList_, this, pkgSize, url);
        }
    }

}

std::vector<std::string> QFrameLessWidget_Alime::GetFilteredVersionKeys(const nlohmann::json& json)
{
    std::vector<std::string> keys;
    if (!mainVersionLocal_.empty()&&json.find(mainVersionLocal_) != json.end())
    {
        for (auto iter = json[mainVersionLocal_].begin(); iter != json[mainVersionLocal_].end(); ++iter)
        {
            std::string key = iter.key();
            //秉承某领导“错了也要能正常工作”的原则
            if (string_utility::startsWith(key.c_str(), "V"))
            {
                key = key.substr(1);
            }
            //这个和业务相关，我们暂时只支持相同注册表的包,也就是V5.x.y只能升级到V5.x.z
            if (AscendingOrder()(versionLocal_.substr(1), key))
            {
                if(IsSameRegKey(versionLocal_.substr(1), key))
                    keys.push_back(iter.key());
            }
        }
        return keys;
    }
    else {
        return {};
    }

}

void QFrameLessWidget_Alime::ReadUpdatePacksInfo()
{
    //
    const nlohmann::json& json = json_["UpdatePacks"];

    std::vector<std::string> keys = GetFilteredVersionKeys(json);
    QNetworkAccessManager manager;
    for (const auto& key : keys)
    {
        std::string webUrl = json[mainVersionLocal_][key];
        QString url = ConfigFileReadWriter::Instance().GetUrlOfUpdatePackFolder() + webUrl.c_str();
        QEventLoop loop;
        QNetworkReply* reply = manager.head(QNetworkRequest(url));
        //我们阻塞当前线程,初始化stackWidget内容
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()), Qt::DirectConnection);
        loop.exec();

        QVariant var = reply->header(QNetworkRequest::ContentLengthHeader);
        if (reply->error())
        {
            qDebug() << reply->errorString();
            reply->deleteLater();
            continue;
        }
        reply->deleteLater();

        auto pkgSize = var.toLongLong();
        QListWidgetItem* item = new QListWidgetItem();
        QSize preferSize = item->sizeHint();
        item->setSizeHint(QSize(preferSize.width(), 70));
        updatePkgList_->addItem(item);
        auto index = url.lastIndexOf("/");
        auto itemWidget = new DownloadInfoWidget(this, url.mid(index + 1), pkgSize, url);
        updatePkgList_->setItemWidget(item, itemWidget);
    }
}

void QFrameLessWidget_Alime::ShowVersionTipsInfo(const QString& str)
{
    versionTips_->setText(str);
}

void QFrameLessWidget_Alime::SetTips(const QString& str, bool IsColorRed)
{
    if(!IsColorRed)
        ShowVersionTipsInfo(str);
    else
    {
        QString strHTML = QString("<html><head><style> #f{font-size:18px; color: red;} /style></head>\
                            <body><font id=\"f\">%1</font></body></html>").arg(str);
        ShowVersionTipsInfo(strHTML);
    }
}
