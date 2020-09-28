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
    : Alime_ContentWidget(parent)
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
    
    QWidget* leftButtons = new QWidget(this);//��ť������
    leftLayout->addWidget(leftButtons);
    QWidget* dummy = new QWidget(this);//�����һ��ռλ
    leftLayout->addWidget(dummy);
    leftLayout->setStretch(0, 2);
    leftLayout->setStretch(1, 3);

    ///add left
    ///add left button
    QVBoxLayout* leftButtonLayout = new QVBoxLayout(leftButtons);
    leftButtonLayout->setMargin(0);
    leftButtonLayout->setSpacing(0);
    leftButtonLayout->addSpacing(20);

    QPushButton* btn01 = new QPushButton(u8"������");
    btn01->setObjectName("btnBoard");
    btn01->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btn01->setCheckable(true);
    btn01->setChecked(true);
    leftButtonLayout->addWidget(btn01);

    QPushButton* btn02 = new QPushButton(u8"������");
    btn02->setObjectName("btnBoard");
    btn02->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btn02->setCheckable(true);
    leftButtonLayout->addWidget(btn02);

    QPushButton* btn03 = new QPushButton(u8"���°����");
    btn03->setObjectName("btnBoard");
    btn03->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btn03->setCheckable(true);
    leftButtonLayout->addWidget(btn03);

    QButtonGroup* group = new QButtonGroup(this);
    group->addButton(btn01,0);
    group->addButton(btn02,1);
    group->addButton(btn03,2);
    
    //leftContent->setFixedWidth(240);

    //�����ұ߾���vbox�����벼����Ϊ�˱���һ��������ԣ������Ժ������������(����)��
    rightContent_->setObjectName("rightContent");
    QVBoxLayout* vbox = new QVBoxLayout(rightContent_);

    versionTips_ = new QLabel();
    versionTips_->setObjectName("versionTips");

    if (!versionLocal_.empty())
    {
        ////fix me
        versionTips_->setText((u8"��鵽��ǰ�汾:" + versionLocal_ + "   " + u8"�ҵ����¿������汾").c_str());
    }
    else
    {
        QString strHTML = QString("<html><head><style> #f{font-size:18px; color: red;} /style></head>\
                            <body><font id=\"f\">%1</font></body></html>").arg(u8"�޷���ѯ��ǰ�汾��Ϣ, ��ӹ�������");
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

    //��ʹ��ID�ǵ��Ŀ���Ҫ����˳�� 0.0
    connect(group, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
        stackWidget_, &QStackedWidget::setCurrentIndex);
    connect(group, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
        [=]() {
            LocalVersionFile finder;
            finder.SetVersionFileFolder(QApplication::applicationDirPath().toLocal8Bit().data());
            QString version = QString("V")+ finder.GetLocalVersion().c_str();
            versionTips_->setText(u8"��鵽��ǰ�汾:" + version);
            
            if (version != versionLocal_.c_str())
            {
                updatePkgList_->clear();
                fixPkgList_->clear();
                ReadUpdatePacksInfo();//fix me,ȥ������
                ReadFixPacksInfo();//fix me
                ReadInstallationCDInfo();
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
        QString strHTML = QString("<html><head><style> #f{font-size:18px; color: red;} /style></head>\
                            <body><font id=\"f\">%1</font></body></html>").arg(u8"�޷���鵽����������Ϣ����������");
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
    return u8"Pkpm��������";
}

bool QFrameLessWidget_Alime::ReadLocalVersion()
{
    auto versionFiles = FindSpecificFiles::FindVersionFiles(QApplication::applicationDirPath().toLocal8Bit().data(), "V", "ini");
    if (versionFiles.size() == 0)
    {
        qWarning() << u8"�޷��ҵ��汾��Ϣ�ļ�";
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

//����������������
bool QFrameLessWidget_Alime::InitDownloadList(const std::string& str)
{
    if (!ReadLocalVersion())
    {
        //fix me, show info "�޷���ȡ���ذ汾��Ϣ"
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
    const std::string isoFileUrl= json_["LatestIsoUrl"];
    QNetworkAccessManager manager;
    QString url(isoFileUrl.c_str());
    QEventLoop loop;
    QNetworkReply* reply = manager.head(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()), Qt::DirectConnection);
    loop.exec();
    QVariant var = reply->header(QNetworkRequest::ContentLengthHeader);
    if (reply->error())
    {
        qDebug() << reply->errorString();
        reply->deleteLater();
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

//copy�˴��룬�������õ�ζ����
//��Ҫ����
void QFrameLessWidget_Alime::ReadFixPacksInfo()
{
    const nlohmann::json& json = json_["FixPacks"];

    if (!versionLocal_.empty() && json.find(versionLocal_) != json.end())
    {
        nlohmann::json array = json_["FixPacks"][versionLocal_];
        QNetworkAccessManager manager;//���ϵ���˼�����5������
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
            //����ĳ�쵼������ҲҪ��������������ԭ��
            if (string_utility::startsWith(key.c_str(), "V"))
            {
                key = key.substr(1);
            }
            if (AscendingOrder()(versionLocal_.substr(1), key))
            {
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
        //����������ǰ�߳�,��ʼ��stackWidget����
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
