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

//#include <algorithm>

#include "Alime/Console.h"

#include "QFrameLessWidget_Alime.h"
#include "VersionFileFinder.h"
#include "DownloadInfoWidget.h"
#include "ConfigFileRW.h"
#include "PackageListWidget.h"


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
    pkgList_(nullptr),
    leftContent_(nullptr),
    rightContent_(nullptr),
    versionTips_(nullptr)
{
    versionTips_ = new QLabel();
    versionTips_->setObjectName("versionTips");

    ReadLocalVersion();

    QHBoxLayout* contentLayout = new QHBoxLayout(this);
    leftContent_ = new QWidget(this);
    leftContent_->setObjectName("leftContent");
    leftContent_->setVisible(false);

    rightContent_ = new QWidget(this);
    rightContent_->setObjectName("rightContent");

    contentLayout->addWidget(leftContent_);
    contentLayout->addWidget(rightContent_);
    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);
    contentLayout->setStretch(0, 1);
    contentLayout->setStretch(1, 4);

    QVBoxLayout* leftLayout = new QVBoxLayout(leftContent_);
    leftLayout->setMargin(0); 
    
    QWidget* leftButtonsWidget = new QWidget(this);
    leftLayout->addWidget(leftButtonsWidget);
    leftLayout->addStretch();

    QVBoxLayout* leftButtonLayout = new QVBoxLayout(leftButtonsWidget);
    leftButtonLayout->setMargin(0);
    leftButtonLayout->setSpacing(0);
    leftButtonLayout->addSpacing(20);

    MAKE_PUSHBUTTON(btn01, u8"��ѡ������", "btnBoard", true, true, leftButtonLayout);
    MAKE_PUSHBUTTON(btn02, u8"�������/����", "btnBoard", true, false, leftButtonLayout);

    QButtonGroup* group = new QButtonGroup(this);
    group->addButton(btn01, 0);
    group->addButton(btn02, 1);

    QVBoxLayout* vbox = new QVBoxLayout(rightContent_);
    vbox->addWidget(versionTips_);

    {
        //QFrame* line = new QFrame(this);
        //line->setObjectName("horiLine");
        //line->setFrameShape(QFrame::HLine);
        //vbox->addWidget(line);
    }

    pkgList_ = new PackageListWidget(this);
    pkgList_->SetMainWidget(this);
    stackWidget_ = new QStackedWidget(this);
    stackWidget_->addWidget(pkgList_);
    stackWidget_->addWidget(new QListWidget(this));

    //һ�����ص���壬���ڲ���
    connect(group, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
        [=](int index) {
            auto sz = stackWidget_->count();
            if (index >= sz)
            {
                QMessageBox::critical(NULL, "error", "invalid stackwidget index ", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                std::abort();
            }
            stackWidget_->setCurrentIndex(index);
            return;
        });

    vbox->addWidget(stackWidget_);
    vbox->addSpacing(40);

    ReadPkgFileInfo();
}

void QFrameLessWidget_Alime::ReadPkgFileInfo()
{
    auto &config=ConfigFileReadWriter::Instance();

    //�ṩ����packinfo.json����, ����Դ���������ϡ�Ӧ��oss�����˻����ܵ�¼�������
    if (config.IsLocalPackFileInfoOn())
    {
        QFile f(config.GetLocalPackInfoPath());
        if (f.open(QIODevice::ReadOnly))
        {
            auto jsonStr=f.readAll().toStdString();
            if (!jsonStr.empty())
            {
                json_= nlohmann::json::parse(jsonStr);
                InitDownloadList();
                return;
            }
                
        }
    }
    /*
        ��Ϊ��ɫ����ԭ������ѡ��֧��https�����ѡ����ǰ���: �������IOʱ��dll���е��ˡ�
        ��ô���ʣ�����openssl����©����ô�죿�ǻ�������ҵ�İ�ȫ��ʶ�����һ���Ѿ�
        qDebug() << manager->supportedSchemes();
        openssl 1.1.1, debug�汾Ĭ��֧��openssl, release�汾Ĭ�ϲ�֧��openssl
        bool bSupp = QSslSocket::supportsSsl();
        QString buildVersion = QSslSocket::sslLibraryBuildVersionString();
        QString version = QSslSocket::sslLibraryVersionString();
        qDebug() << bSupp << buildVersion << version << endl;
    */
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &QFrameLessWidget_Alime::QueryInfoFinish);
    connect(manager, &QNetworkAccessManager::finished, manager, &QObject::deleteLater);

    auto url = QUrl(ConfigFileReadWriter::Instance().GetUrlOfUpdateInfoFile());
    manager->get(QNetworkRequest(url));
}

void QFrameLessWidget_Alime::QueryInfoFinish(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "Error Happened";
        SetTips(u8"�޷���鵽����������Ϣ��������������", true);
        netAvailable_ = false;
    }
    else
    {
        netAvailable_ = true;
        const QByteArray reply_data = reply->readAll();
        QString str(reply_data);
        json_ = nlohmann::json::parse(str.toStdString());
        bool ret=InitDownloadList();
        if(!ret)
            SetTips(u8"�޷���鵽����������Ϣ��������������", true);
    }
}

QString QFrameLessWidget_Alime::GetTitle()
{
    return u8"Pkpm��������";
}

bool QFrameLessWidget_Alime::ReadLocalVersion()
{
    auto versionFiles = FindSpecificFiles::FindVersionFiles(
        (QApplication::applicationDirPath()+"/../CFG/").toLocal8Bit().data(), "V", "ini");
    if (versionFiles.size() == 0)
    {
        qWarning() << u8"�޷��ҵ��汾��Ϣ�ļ�";
    }
    else
    {
        std::sort(versionFiles.begin(), versionFiles.end(), AscendingOrder());
        versionLocal_ = "V" + versionFiles.back();
        mainVersionLocal_ = "V";
        mainVersionLocal_.push_back(versionFiles.back().front());
        mainVersionLocal_.push_back(versionFiles.back()[2]);
        
        Alime::Console::WriteLine(L"��ȡ�����ذ汾:"+QString::fromLocal8Bit(versionLocal_.c_str()).toStdWString());
        Alime::Console::WriteLine(L"��ȡ�����ذ汾��Ӧע���:" + QString::fromLocal8Bit(mainVersionLocal_.c_str()).toStdWString());
    }

    if (!versionLocal_.empty())
        SetTips((u8"��鵽��ǰ�汾:" + versionLocal_ + "   " + u8"�ҵ����¿������汾").c_str());
    else
        SetTips(u8"�޷���ѯ���ذ汾��Ϣ, ��ӹ�������������������");
    return true;
}

bool QFrameLessWidget_Alime::InitDownloadList()
{
    try
    {
        if (!ReadLocalVersion())
        {
            versionTips_->setText(u8"�޷���ȡ���ذ汾��Ϣ");
            return false;
        }
        
        pkgList_->SetVersion(mainVersionLocal_, versionLocal_);
        pkgList_->clear();

        std::string lastestVer = json_["LatestVersion"];
        Alime::Console::WriteLine((L"�������°汾:" + QString(lastestVer.c_str()).toStdWString()).c_str());

        if (AscendingOrder().Compare(versionLocal_, lastestVer) >= 0)
        {
            Alime::Console::WriteLine(L"�������°汾");
            SetTips(u8"�������°汾");
        }
        else
            pkgList_->Parse(json_);
    }
    catch (...)
    {
        qWarning() << "can not parse info from webInfo.json";
        return false;
    }
    return true;
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
    //    qDebug() << u8"��ѯ������Ϣʱ����";
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

    ComparisonDownloadInfoWidget* itemWidget = new ComparisonDownloadInfoWidget(this, version);
    return true;
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
