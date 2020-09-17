#include <QLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QMessageBox>
#include <QUrl>
#include <QApplication>

#include "QtNetwork/qnetworkaccessmanager.h"
#include "QtNetwork/qnetworkrequest.h"
#include "QtNetwork/qnetworkreply.h"

#include "DownloadInfoWidget.h"
#include "QFrameLessWidget_Alime.h"
#include "VersionFileFinder.h"
#include "AppVersion.h"
#include "thirdParty/nlohmann/json.hpp"

CLASSREGISTER(QFrameLessWidget_Alime)

QFrameLessWidget_Alime::QFrameLessWidget_Alime(QWidget* parent)
    : Alime_ContentWidget(parent)
{
    {
        ReadPkgFileInfo();
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
    {
        QWidget* leftButtons = new QWidget(this);
        leftLayout->addWidget(leftButtons);
        QWidget* dummy = new QWidget(this);
        leftLayout->addWidget(dummy);
        leftLayout->setStretch(0, 1);
        leftLayout->setStretch(1, 1);

        QVBoxLayout* leftButtonLayout = new QVBoxLayout(leftButtons);
        leftButtonLayout->setMargin(0);
        leftButtonLayout->addSpacing(20);

        QPushButton* btn01 = new QPushButton(u8"补丁包");
        btn01->setObjectName("btnBoard");
        btn01->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        leftButtonLayout->addWidget(btn01);

        QPushButton* btn02 = new QPushButton(u8"光盘");
        btn02->setObjectName("btnBoard");
        leftButtonLayout->addWidget(btn02);
        btn02->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        QPushButton* btn03 = new QPushButton("click me");
        btn03->setObjectName("btnBoard");
        btn03->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        leftButtonLayout->addWidget(btn03);

        QPushButton* btn04 = new QPushButton("click me");
        btn04->setObjectName("btnBoard");
        btn04->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        leftButtonLayout->addWidget(btn04);


    }
    //leftContent->setFixedWidth(240);

    //整个右边就是vbox，加入布局是为了保持一定的灵活性，方便以后加入其他内容(竖条)。
    rightContent_->setObjectName("rightContent");
    QVBoxLayout* vbox = new QVBoxLayout(rightContent_);

    versionTips_ = new QLabel(u8"未检查到当前版本, 请检查您的网络");
    versionTips_->setObjectName("versionTips");
    QString version=GetLocalVersion();
    if(!version.isEmpty())
        versionTips_->setText(u8"检查到当前版本:"+version+"   "+u8"找到以下可升级版本");
    vbox->addWidget(versionTips_);

    downloadList_ = new QListWidget(this);
    downloadList_->setObjectName("patchPkgList");
    vbox->addWidget(downloadList_);
}

void QFrameLessWidget_Alime::ReadPkgFileInfo()
{
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &QFrameLessWidget_Alime::QueryInfoFinish);
    qDebug() << manager->supportedSchemes();
    manager->get(QNetworkRequest(QUrl("http://update.pkpm.cn/PKPM2010/Info/pkpmSoft/packageInfo.json")));
}

void QFrameLessWidget_Alime::QueryInfoFinish(QNetworkReply* reply)
{
    auto ret=reply->error();
    if (ret != QNetworkReply::NoError)
    {
        qDebug() << "Error Happened";
        QString strHTML = QString("<html><head><style> #f{font-size:18px; color: red;} /style></head>\
                            <body><font id=\"f\">%1</font></body></html>").arg(u8"无法检查到程序升级信息，请检查网络");
        ShowVersionTipsInfo(strHTML);
    }
    else
    {
        const QByteArray reply_data = reply->readAll();
        QString str(reply_data);
        InitDownloadList(str.toStdString());
    }
}

QString QFrameLessWidget_Alime::GetTitle()
{
    return u8"Pkpm升级程序";
}

//这个函数不允许出错
bool QFrameLessWidget_Alime::InitDownloadList(const std::string& str)
{
    //得到本地程序最新版本号
    auto versionFiles=FindSpecificFiles::FindVersionFiles(QApplication::applicationDirPath().toLocal8Bit().data() , "V", "ini");
    //本地文件被fuck了
    if (versionFiles.size() == 0)
    {
        //fix me
        return false;
    }
    std::sort(versionFiles.begin(), versionFiles.end(), AscendingOrder());
    
    std::string mainVersion="V";
    mainVersion.push_back(versionFiles.back().front());
   
    std::vector<std::string> keys;
    nlohmann::json json= nlohmann::json::parse(str);
    //找到主版本
    if (json.find(mainVersion) != json.end())
    {
        for (auto iter = json[mainVersion].begin(); iter != json[mainVersion].end(); ++iter)
        {
            QNetworkAccessManager manager;
            std::string key = iter.key();
            if (string_utility::startsWith(key.c_str(), "V")) {
                key = key.substr(1);
            }
            if (AscendingOrder()(versionFiles.back(), key))
            {
                keys.push_back(iter.key());
                std::string webUrl = json[mainVersion][iter.key()];
                
                QString url = ("http://update.pkpm.cn/PKPM2010/Info/pkpmSoft/" + webUrl).c_str();
                QEventLoop loop;
                QNetworkReply* reply = manager.head(QNetworkRequest(url));
                //我们阻塞当前线程
                connect(reply, SIGNAL(finished()), &loop, SLOT(quit()), Qt::DirectConnection);
                loop.exec();
                QVariant var = reply->header(QNetworkRequest::ContentLengthHeader);
                int pkgSize=var.toInt();

                QListWidgetItem* item = new QListWidgetItem();
                QSize preferSize = item->sizeHint();
                item->setSizeHint(QSize(preferSize.width(), 70));
                downloadList_->addItem(item);
                auto index=url.lastIndexOf("/");
                auto itemWidget = new DownloadInfoWidget(this, url.mid(index+1) /*QString(iter.key().c_str())*/, pkgSize, url);
                downloadList_->setItemWidget(item, itemWidget);
            }
                
        }
    }
    return true;
}

void QFrameLessWidget_Alime::ShowVersionTipsInfo(const QString& str)
{
    versionTips_->setText(str);
}
