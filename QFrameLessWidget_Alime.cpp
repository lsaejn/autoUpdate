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



#define MAKE_PUSHBUTTON(varName, text, objectName, checkable, checked, layout) \
QPushButton* varName = new QPushButton(text); \
varName->setObjectName(objectName); \
varName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); \
varName->setCheckable(checkable); \
varName->setChecked(checked); \
layout->addWidget(varName)

QFrameLessWidget_Alime::QFrameLessWidget_Alime(QWidget* parent)
    : Alime_ContentWidget(parent),
    netAvailable_(false),
    stackWidget_(nullptr),
    updatePkgList_(nullptr),
    fixPkgList_(nullptr),
    imageWidget_(nullptr),
    leftContent_(nullptr),
    rightContent_(nullptr),
    versionTips_(nullptr)
{
    versionTips_ = new QLabel();
    versionTips_->setObjectName("versionTips");

    ReadLocalVersion();

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

    MAKE_PUSHBUTTON(btn01, u8"升级到最新版本", "btnBoard", true, true, leftButtonLayout);
    MAKE_PUSHBUTTON(btn02, u8"当前版本补丁", "btnBoard", true, false, leftButtonLayout);
    MAKE_PUSHBUTTON(btn03, u8"最新版光盘", "btnBoard", true, false, leftButtonLayout);
    MAKE_PUSHBUTTON(btn04, u8"差异更新/测试", "btnBoard", true, false, leftButtonLayout);
    btn03->setVisible(false);
    btn04->setVisible(false);

    QButtonGroup* group = new QButtonGroup(this);
    group->addButton(btn01, 0);
    group->addButton(btn02, 1);
    group->addButton(btn03, 2);
    group->addButton(btn04, 3);

    //leftContent->setFixedWidth(240);

    //整个右边就是vbox，加入布局是为了保持一定的灵活性，方便以后加入其他内容(竖条)。
    rightContent_->setObjectName("rightContent");
    QVBoxLayout* vbox = new QVBoxLayout(rightContent_);

    vbox->addWidget(versionTips_);

    updatePkgList_ = new SetupWidget(this);
    updatePkgList_->setObjectName("updatePkgList");
    fixPkgList_ = new SetupWidget(this);
    fixPkgList_->setObjectName("fixPkgList");
    imageWidget_ = new SetupWidget(this);
    integralFilesPackList_ = new QListWidget(this);

    stackWidget_ = new QStackedWidget(this);
    stackWidget_->addWidget(updatePkgList_);
    stackWidget_->addWidget(fixPkgList_);
    stackWidget_->addWidget(imageWidget_);
    stackWidget_->addWidget(integralFilesPackList_);

    //fix me, 删掉这个连接。
    //每次更新完成后，重新读版本信息文件，然后刷新列表
    connect(group, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
        [=](int index) {
            {
                for (int i = 0; i != stackWidget_->count(); ++i)
                {
                    SetupWidget* stackElem = dynamic_cast<SetupWidget*>(stackWidget_->widget(i));
                    if (stackElem && stackElem->IsAutoSetupOn()&& i!=index)
                    {
                        ShowWarningBox("error", u8"另一个安装正在执行", u8"确定");
                        auto btn=group->button(i);
                        btn->setChecked(true);
                        return;
                    }
                }
            }
            stackWidget_->setCurrentIndex(index);

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
                integralFilesPackList_->clear();

                ReadLocalVersion();
                ReadUpdatePacksInfo();
                ReadFixPacksInfo();//fix me
                ReadIntegralFilesPackInfo();
            }
        });
    vbox->addWidget(stackWidget_);

    QWidget* foot = new  QWidget(this);
    QHBoxLayout* footBox = new QHBoxLayout(foot);
    QPushButton* updateBtn = new QPushButton(u8"一键更新", foot);

    connect(updateBtn, &QPushButton::clicked, [=]() {
        SetupWidget* stackElem=dynamic_cast<SetupWidget*>(stackWidget_->currentWidget());
        if (!stackElem)
        {
            qWarning() << u8"stackWidget switch a index that should not opened to users";
            return;
        }
        else
        {
            if (!stackElem->IsAutoSetupOn())
                stackElem->SetupAllTask();
            else
            {
                ShowWarningBox("error", u8"另一个安装正在执行", u8"确定");
                return;
            }
            connect(stackElem, &SetupWidget::installing, [=](int i) {
                QString str = QString(u8"正在处理第%1个安装包").arg(i);
                updateBtn->setText(str);
                });
            connect(stackElem, &SetupWidget::finish, [=]() {
                updateBtn->setText(u8"一键更新");
                stackElem->clear();
                ReadLocalVersion();
                ReadPkgFileInfo();
                });
            connect(stackElem, &SetupWidget::error, [=]() {
                updateBtn->setText(u8"升级失败,重新开始");
                });

        }
        });

    updateBtn->setObjectName("updateBtn");
    footBox->addStretch(0);
    footBox->addWidget(updateBtn);
    footBox->addSpacing(50);
    vbox->addWidget(foot);
    vbox->addSpacing(40);

    ReadPkgFileInfo();
}

void QFrameLessWidget_Alime::ReadPkgFileInfo()
{
    auto &config=ConfigFileReadWriter::Instance();
    if (config.IsLocalPackFileInfoOn())
    {
        QFile f(config.GetLocalPackInfoPath());
        if (f.open(QIODevice::ReadOnly))
        {
            QString qstr=f.readAll();
            if(!qstr.isEmpty()&& InitDownloadList(qstr.toStdString()))
                return;
        }
    }
    //else
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &QFrameLessWidget_Alime::QueryInfoFinish);
    //fix me
    connect(manager, &QNetworkAccessManager::finished, manager, &QObject::deleteLater);
    /*
        qDebug() << manager->supportedSchemes();
        openssl 1.1.1, debug版本默认支持openssl, release版本默认不支持openssl
        bool bSupp = QSslSocket::supportsSsl();
        QString buildVersion = QSslSocket::sslLibraryBuildVersionString();
        QString version = QSslSocket::sslLibraryVersionString();
        qDebug() << bSupp << buildVersion << version << endl;
    */
    auto url = QUrl(ConfigFileReadWriter::Instance().GetUrlOfUpdateInfoFile());
    manager->get(QNetworkRequest(url));
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
    }
    else
    {
        std::sort(versionFiles.begin(), versionFiles.end(), AscendingOrder());
        versionLocal_ = "V" + versionFiles.back();
        mainVersionLocal_ = "V";
        mainVersionLocal_.push_back(versionFiles.back().front());
        //mainVersionLocal_.push_back(versionFiles.back()[2]);
    }

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
    return true;
}

//这个函数不允许出错
bool QFrameLessWidget_Alime::InitDownloadList(const std::string& str)
{
    try
    {
        if (!ReadLocalVersion())
        {
            versionTips_->setText(u8"无法读取本地版本信息");
            return false;
        }

        json_ = nlohmann::json::parse(str);
        
        fixPkgList_->clear();
        updatePkgList_->clear();
        integralFilesPackList_->clear();

        ReadUpdatePacksInfo();
        ReadFixPacksInfo();
        ReadInstallationCDInfo();
        ReadIntegralFilesPackInfo();
    }
    catch (...)
    {
        //fatal, show info and close application
        qWarning() << "can not parse info from webInfo.json";
        return false;
    }
    return true;
}

void QFrameLessWidget_Alime::ReadInstallationCDInfo()
{
    return ReadInstallationCDInfo(imageWidget_);
}

void QFrameLessWidget_Alime::ReadIntegralFilesPackInfo()
{
    auto dubugString = json_.dump();
    try
    {
        nlohmann::json versions = json_["IntegralImageFiles"]["Version"];

        //debug
        QVector<QString> vec;

        for (size_t i = 0; i != versions.size(); ++i)
        {
            std::string v= versions[i].get<std::string>().c_str();
            if (string_utility::startsWith(v.c_str(), "V")&& v!= versionLocal_)
            {
                vec.push_back(QString(v.c_str()));
            }
        }

        for (int i = 0; i != vec.size(); ++i)
            AddItemToComparisonDownloadWidget(vec[i]);
        //fix me
    }
    catch (...)
    {
        qDebug() << "fuck what happen";
        return;
    }
  
    //QNetworkAccessManager manager;
    //QString url(isoFileUrl.c_str());
    //QEventLoop loop;
    //QNetworkReply* reply = manager.head(QNetworkRequest(url));
    //connect(reply, SIGNAL(finished()), &loop, SLOT(quit()), Qt::DirectConnection);
    //loop.exec();
    //QVariant var = reply->header(QNetworkRequest::ContentLengthHeader);
    //if (reply->error())
    //{
    //    qDebug() << u8"查询光盘信息时出错";
    //    qDebug() << reply->errorString();
    //    reply->deleteLater();
    //    return;
    //}
    //reply->deleteLater();

    //qint64 pkgSize = var.toLongLong();
    //AddNewItemAndWidgetToList(imageWidget_, this, pkgSize, url);
}

#include "alternative/FolderDownloadInfoWidget.h"
bool QFrameLessWidget_Alime::AddItemToComparisonDownloadWidget(const QString& version)
{
    QListWidgetItem* item = new QListWidgetItem();
    QSize preferSize = item->sizeHint();
    item->setSizeHint(QSize(preferSize.width(), 40));
    integralFilesPackList_->addItem(item);

    ComparisonDownloadInfoWidget* itemWidget = new ComparisonDownloadInfoWidget(this, version);
    integralFilesPackList_->setItemWidget(item, itemWidget);
    return true;
}

DownloadInfoWidget* QFrameLessWidget_Alime::AddNewItemAndWidgetToList(QListWidget* target, QWidget* /*_parent*/,
    qint64 _fileSize, const QUrl& _url, const QString& fileName)
{
    QListWidgetItem* item = new QListWidgetItem();
    QSize preferSize = item->sizeHint();
    item->setSizeHint(QSize(preferSize.width(), 70));
    target->addItem(item);
    //auto index = _url.lastIndexOf("/");
    auto itemWidget = new DownloadInfoWidget(this, fileName, _fileSize, _url);
    target->setItemWidget(item, itemWidget);
    return itemWidget;
}

//copy了代码，带来不好的味道。
//需要测试
void QFrameLessWidget_Alime::ReadFixPacksInfo()
{
    return ReadFixPacksInfoOfSpecificVersion(fixPkgList_, versionLocal_);
}

void QFrameLessWidget_Alime::ReadInstallationCDInfo(SetupWidget* wgt)
{
    auto dubugString = json_.dump();
    std::string isoFileUrl = json_["LatestIsoUrl"];
    //使用qt https需要额外的库, 对于维护xp机器的人很痛苦
    if (string_utility::startsWith(isoFileUrl.c_str(), "https"))
    {
        isoFileUrl.erase(4, 1);
    }
    QNetworkAccessManager manager;
    QUrl url(isoFileUrl.c_str());
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
    auto itemWidget=AddNewItemAndWidgetToList(wgt, this, pkgSize, url, GetFilePart(url));
    itemWidget->SetCheckCallBack(std::bind(&SetupWidget::IsAutoSetupOn, updatePkgList_));
    itemWidget->SetPackFlag(true);
}

//无比保证这个版本文件是英文
bool IsFileExist(const std::string& filename)
{
    auto str= GetApplicationDirPath() + "../CFG/" + filename.c_str();
    QFile f(str);
    return f.exists();
}


void QFrameLessWidget_Alime::ReadFixPacksInfoOfSpecificVersion(SetupWidget* wgt, const std::string& version)
{
    auto debug = json_.dump(4);
    const nlohmann::json& json = json_["FixPacks"];

    if (!version.empty() && json.find(version) != json.end())
    {
        nlohmann::json array = json_["FixPacks"][version];
        auto sz = array.size();//sz must equals 1, 由于逻辑大改
        QNetworkAccessManager manager;//网上的意思是最多5个请求
        if (true)
        {
            if (IsFileExist(array["patchName"]))
            {
                return;
            }
            auto packName = array["url"].get<std::string>();
            QString url = ConfigFileReadWriter::Instance().GetUrlOfFixPackFolder() + packName.c_str();
            QEventLoop loop;
            QUrl qUrl{ url };
            QNetworkReply* reply = manager.head(QNetworkRequest(qUrl));
            connect(reply, SIGNAL(finished()), &loop, SLOT(quit()), Qt::DirectConnection);
            loop.exec();

            QVariant var = reply->header(QNetworkRequest::ContentLengthHeader);
            if (reply->error())
            {
                qDebug() << reply->errorString();
                reply->deleteLater();
                return;
            }
            reply->deleteLater();

            int pkgSize = var.toInt();
            //auto fullName = qUrl.toString();
            //auto fileApart = GetFilePart(fullName);
            auto item = AddNewItemAndWidgetToList(wgt, this, pkgSize, url, GetFilePart(qUrl));
            item->SetCheckCallBack(std::bind(&SetupWidget::IsAutoSetupOn, wgt));
            item->SetPackFlag(false);
            if (versionLocal_ != version)
            {
                item->isInWrongPosition_ = true;
                //fix me, use [&]
                connect(item, &DownloadInfoWidget::finishSetup, [=](bool isUpdatePack) {
                    if (isUpdatePack == false && updatePkgList_->IsAutoSetupOn())
                    {
                        updatePkgList_->clear();
                        fixPkgList_->clear();
                        integralFilesPackList_->clear();
                        ReadLocalVersion();
                        ReadUpdatePacksInfo();
                        ReadFixPacksInfo();//fix me
                        {
                            auto hwnd = (HWND)window()->winId();
                            RECT rc;
                            ::GetWindowRect(hwnd, &rc);
                            MoveWindow(hwnd, rc.left, rc.top, rc.right - rc.left - 1, rc.bottom - rc.top - 1, 1);
                            MoveWindow(hwnd, rc.left, rc.top, rc.right - rc.left + 1, rc.bottom - rc.top + 1, 1);
                        }
                    }
                    });
            }
            else
            {
                connect(item, &DownloadInfoWidget::finishSetup, [&](bool isUpdatePack) {
                    fixPkgList_->clear();
                    ReadFixPacksInfo();
                    auto hwnd = (HWND)window()->winId();
                    RECT rc;
                    ::GetWindowRect(hwnd, &rc);
                    MoveWindow(hwnd, rc.left, rc.top, rc.right - rc.left - 1, rc.bottom - rc.top - 1, 1);
                    MoveWindow(hwnd, rc.left, rc.top, rc.right - rc.left + 1, rc.bottom - rc.top + 1, 1);
                    });
            }
                
            
        }
    }
}

#include <algorithm>
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
            auto noPrefix=versionLocal_.substr(1);
            if (AscendingOrder()(noPrefix, key)&& IsSameRegKey(noPrefix, key))
            {
                    keys.push_back(iter.key());
            }
        }
        //需求改了，但是要求是更严格了。所以我们在这里再过滤一次
        //我们只给出最新的包
        std::sort(keys.begin(), keys.end(), AscendingOrder());
        if (keys.size() > 1)
        {
            keys.erase(keys.begin(), --keys.end());
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
        QUrl url = ConfigFileReadWriter::Instance().GetUrlOfUpdatePackFolder() + webUrl.c_str();
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

        auto itemWidget = new DownloadInfoWidget(this, GetFilePart(url), pkgSize, url);
        updatePkgList_->setItemWidget(item, itemWidget);
        itemWidget->SetCheckCallBack(std::bind(&SetupWidget::IsAutoSetupOn, updatePkgList_));

        connect(itemWidget, &DownloadInfoWidget::finishSetup, [&](bool isUpdatePack) {
            if (!itemWidget)
            {
                int x = 3;//fuck
            }
            if ((isUpdatePack &&!updatePkgList_->IsAutoSetupOn()))
            {
                updatePkgList_->clear();
                fixPkgList_->clear();
                integralFilesPackList_->clear();
                ReadLocalVersion();
                ReadUpdatePacksInfo();
                ReadFixPacksInfo();//fix me
                //这个地方看着很难堪~显然Qt应该要做到像wpf一样，updatePkgList_=null, updatePkgList_=...
                {
                    auto hwnd = (HWND)window()->winId();
                    RECT rc;
                    ::GetWindowRect(hwnd, &rc);
                    MoveWindow(hwnd, rc.left, rc.top, rc.right - rc.left - 1, rc.bottom - rc.top - 1, 1);
                    MoveWindow(hwnd, rc.left, rc.top, rc.right - rc.left + 1, rc.bottom - rc.top + 1, 1);
                }
            }
            });
        //10/28, 好爽
        ReadFixPacksInfoOfSpecificVersion(updatePkgList_, key);
    }

    //10/27 我们留到以后再改这个地方
    auto num = keys.size();
    if (num == 0&& json.find(mainVersionLocal_) == json.end())
    {
        //光盘
        ReadInstallationCDInfo(updatePkgList_);
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
