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


//!
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
        leftButtonLayout->setSpacing(0);
        leftButtonLayout->addSpacing(20);

        QPushButton* btn01 = new QPushButton(u8"补丁包");
        btn01->setObjectName("btnBoard");
        btn01->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        leftButtonLayout->addWidget(btn01);
        btn01->setCheckable(true);
        btn01->setChecked(true);

        QPushButton* btn02 = new QPushButton(u8"光盘");
        btn02->setObjectName("btnBoard");
        leftButtonLayout->addWidget(btn02);
        btn02->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btn02->setCheckable(true);

        QPushButton* btn03 = new QPushButton(u8"升级包");
        btn03->setObjectName("btnBoard");
        btn03->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        leftButtonLayout->addWidget(btn03);
        btn03->setCheckable(true);

        QButtonGroup* group = new QButtonGroup(this);
        group->addButton(btn01); group->addButton(btn02); group->addButton(btn03);
    }
    //leftContent->setFixedWidth(240);

    //整个右边就是vbox，加入布局是为了保持一定的灵活性，方便以后加入其他内容(竖条)。
    rightContent_->setObjectName("rightContent");
    QVBoxLayout* vbox = new QVBoxLayout(rightContent_);

    versionTips_ = new QLabel();
    versionTips_->setObjectName("versionTips");


    LocalVersionFile finder;
    finder.SetVersionFileFolder(QApplication::applicationDirPath().toLocal8Bit().data());
    QString version= finder.GetLocalVersion().c_str();
    if (!version.isEmpty())
    {
        versionTips_->setText(u8"检查到当前版本:" + version + "   " + u8"找到以下可升级版本");
    }
    else
    {
        QString strHTML = QString("<html><head><style> #f{font-size:18px; color: red;} /style></head>\
                            <body><font id=\"f\">%1</font></body></html>").arg(u8"无法查询当前版本信息, 请从官网下载");
        ShowVersionTipsInfo(strHTML);
    }

    vbox->addWidget(versionTips_);

    updatePkgList_ = new QListWidget(this);
    updatePkgList_->setObjectName("patchPkgList");

    stackWidget_ = new QStackedWidget(this);
    stackWidget_->addWidget(updatePkgList_);
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

bool QFrameLessWidget_Alime::ReadLocalVersion()
{
    auto versionFiles = FindSpecificFiles::FindVersionFiles(QApplication::applicationDirPath().toLocal8Bit().data(), "V", "ini");
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
}

//这个函数不允许出错
bool QFrameLessWidget_Alime::InitDownloadList(const std::string& str)
{
    if (ReadLocalVersion())
    {
        //fix me, show info "无法读取本地版本信息"
        return false;
    }
    //ReadInstallationCDInfo(const std::string & info);
    //ReadFixPacksInfo(const std::string & info);
    nlohmann::json json = nlohmann::json::parse(str);
    ReadUpdatePacksInfo(json);
   


    return true;
}

void QFrameLessWidget_Alime::ReadInstallationCDInfo(const nlohmann::json& json)
{

}

void QFrameLessWidget_Alime::ReadFixPacksInfo(const nlohmann::json& json)
{

}

void QFrameLessWidget_Alime::ReadUpdatePacksInfo(const nlohmann::json& info)
{
    //
    const nlohmann::json& json = info["UpdatePacks"];

    std::vector<std::string> keys;

    //考虑到这里低素质的维护人员可能连顺序都会写错,我还得TMD帮你排序
    if (json.find(mainVersionLocal_) != json.end())
    {
        QNetworkAccessManager manager;
        for (auto iter = json[mainVersionLocal_].begin(); iter != json[mainVersionLocal_].end(); ++iter)
        { 
            std::string key = iter.key();
            //秉承某领导“错了也要能正常工作”的原则
            if (string_utility::startsWith(key.c_str(), "V"))
            {
                key = key.substr(1);
            }

            if (AscendingOrder()(versionLocal_.substr(1), key))
            {
                keys.push_back(iter.key());
                std::string webUrl = json[mainVersionLocal_][iter.key()];
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
                int pkgSize = var.toInt();

                QListWidgetItem* item = new QListWidgetItem();
                QSize preferSize = item->sizeHint();
                item->setSizeHint(QSize(preferSize.width(), 70));
                updatePkgList_->addItem(item);
                auto index = url.lastIndexOf("/");
                auto itemWidget = new DownloadInfoWidget(this, url.mid(index + 1), pkgSize, url);
                updatePkgList_->setItemWidget(item, itemWidget);
            }
            else
            {
                qWarning() << "may some error was raised, config file was error or bad version";
            }
        }
    }
    else
    {
        //找不到当前大版本~~牛逼
    }
}


void QFrameLessWidget_Alime::ShowVersionTipsInfo(const QString& str)
{
    versionTips_->setText(str);
}
