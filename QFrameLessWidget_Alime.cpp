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
#include "PackageListWidget.h"
#include "Alime/Console.h"

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
    //leftContent_->setVisible(false);
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

    stackWidget_ = new QStackedWidget(this);
    pkgList_ = new PackageListWidget(this);

    stackWidget_->addWidget(pkgList_);
    stackWidget_->addWidget(new QListWidget(this));

    //fix me, ɾ��������ӡ�
    //ÿ�θ�����ɺ����¶��汾��Ϣ�ļ���Ȼ��ˢ���б�
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

            LocalVersionFile finder;
            auto newVersion=finder.GetLocalVersion();
            if (newVersion.empty())
                return;
            QString version = QString("V")+ newVersion.c_str();
            //versionTips_->setText(u8"��鵽��ǰ�汾:" + version);
            if (netAvailable_)
            {
                QString tips= version+ u8"   �ҵ����¿�����Ϣ";
                versionTips_->setText(u8"��鵽��ǰ�汾:" + tips);
            }
            else
                return;
            //fix me, û�в��Թ���һ��
            if (version != versionLocal_.c_str())
            {
                pkgList_->clear();
                ReadLocalVersion();
                ReadUpdatePacksInfo();
                ReadFixPacksInfo();//fix me
                ReadIntegralFilesPackInfo();
            }
        });
    vbox->addWidget(stackWidget_);
    vbox->addSpacing(40);

    ReadPkgFileInfo();
}

void QFrameLessWidget_Alime::ReadPkgFileInfo()
{
    auto &config=ConfigFileReadWriter::Instance();

    //�ṩ����packinfo.json����, ��Դ���������ϡ�oss�����˻����ܵ�¼�������
    if (config.IsLocalPackFileInfoOn())
    {
        QFile f(config.GetLocalPackInfoPath());
        if (f.open(QIODevice::ReadOnly))
        {
            QString qstr=f.readAll();
            if(!qstr.isEmpty() && InitDownloadList(qstr.toStdString()))
                return;
        }
    }
    /*
    ��Ϊ��ɫ����ԭ�����ǲ�֧��https��
    ���ѡ����ǰ���: �������IOʱ��dll���е��ˡ�
    ��ô���ʣ�����openssl��©����ô�죿
    ����˵�ˡ��ǻ�������ҵ�İ�ȫ��ʶ���ǡ�
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
        InitDownloadList(str.toStdString());
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
    {
        SetTips((u8"��鵽��ǰ�汾:" + versionLocal_ + "   " + u8"�ҵ����¿������汾").c_str());
    }
    else
    {
        SetTips(u8"�޷���ѯ���ذ汾��Ϣ, ��ӹ�������������������");
    }
    return true;
}

//����������������
bool QFrameLessWidget_Alime::InitDownloadList(const std::string& str)
{
    try
    {
        if (!ReadLocalVersion())
        {
            versionTips_->setText(u8"�޷���ȡ���ذ汾��Ϣ");
            return false;
        }

        json_ = nlohmann::json::parse(str);
        
        pkgList_->clear();
        std::string lastestVer = json_["LatestVersion"];
        {
            auto ret = AscendingOrder().Compare(versionLocal_, lastestVer);
            if (ret >= 0)
            {
                Alime::Console::WriteLine(L"�������°汾");
            }
            else
            {
                ReadUpdatePacksInfo();
                ReadFixPacksInfo();
            }
        }
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
    return ReadInstallationCDInfo(NULL);
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

//copy�˴��룬�������õ�ζ����
//��Ҫ����
void QFrameLessWidget_Alime::ReadFixPacksInfo()
{
    ReadFixPacksInfoOfSpecificVersion(pkgList_, versionLocal_);
    return;
}

void QFrameLessWidget_Alime::ReadInstallationCDInfo(PackageListWidget* wgt)
{
    auto dubugString = json_.dump();
    std::string isoFileUrl = json_["LatestIsoUrl"];
    //ʹ��qt https��Ҫ����Ŀ�, ����ά��xp�������˺�ʹ��
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
        qDebug() << u8"��ѯ������Ϣʱ����";
        qDebug() << reply->errorString();
        reply->deleteLater();
        return;
    }
    reply->deleteLater();

    qint64 pkgSize = var.toLongLong();
    wgt->AddItem(this, pkgSize, url, GetFilePart(url));

}

//menu�ļ���֤����汾�ļ���Ӣ��
bool IsFileExist(const std::string& filename)
{
    auto str= GetApplicationDirPath() + "../CFG/" + filename.c_str();
    QFile f(str);
    return f.exists();
}


void QFrameLessWidget_Alime::ReadFixPacksInfoOfSpecificVersion(PackageListWidget* wgt, const std::string& version)
{
    auto debug = json_.dump(4);
    const nlohmann::json& json = json_["FixPacks"];

    if (!version.empty() && json.find(version) != json.end())
    {
        nlohmann::json array = json_["FixPacks"][version];
        auto sz = array.size();//sz must equals 1, �����߼����
        QNetworkAccessManager manager;//���ϵ���˼�����5������
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
            
            auto item = wgt->AddItem(this, pkgSize, url, GetFilePart(qUrl));
            if (versionLocal_ != version)
            {
                item->isInWrongPosition_ = true;
                //fix me, use [&]
                connect(item, &DownloadInfoWidget::finishSetup, [=](bool isUpdatePack) {
                    if (isUpdatePack == false)
                    {
                        pkgList_->clear();

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
                    pkgList_->clear();
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
void QFrameLessWidget_Alime::GetUpdatePackUrl(const nlohmann::json& json, 
    std::string& urlOut, bool& showImage)
{
    std::string ver = json["version"];
    std::string url = json["url"];
    //��reg����ΪV6���ܲ���Ҫע����൱��һ��bool
    bool hasRegStr = false;
    std::string regKey = (hasRegStr=json.contains("reg"))? json["reg"] : "";

    //V5֮ǰ �� ֮��Ĵ�汾���ܻ��Ʋ�ͬ��f...u...c...k
    //����ôԤ��δ��?

    if (mainVersionLocal_.find("V5") != std::string::npos)//��ΪV5֮ǰû�Զ�����
    {
        //ע���ͬ��ֱ�Ӹ�����
        if (mainVersionLocal_ != regKey)
        {
            showImage=true;
            return;
        }
        else
        {
            Alime::Console::WriteLine(L"ע���ͬ");
            auto ret = AscendingOrder().Compare(versionLocal_, ver);
            if (ret>=0)
            {
                Alime::Console::WriteLine(L"�������°汾");
                return ;
            }
            else
            {
                urlOut = url;
                return;
            }
        }
    }
    else
    {

    }

    std::vector<std::string> keys;
    if (!mainVersionLocal_.empty() &&
        json.find(mainVersionLocal_) != json.end())
    {
        for (auto iter = json[mainVersionLocal_].begin();
            iter != json[mainVersionLocal_].end(); ++iter)
        {
            std::string key = iter.key();
            //����ĳ�쵼������ҲҪ��������������ԭ��
            if (string_utility::startsWith(key.c_str(), "V"))
            {
                key = key.substr(1);
            }
            //�����ҵ����أ�������ʱֻ֧����ͬע���İ�,Ҳ����V5.x.yֻ��������V5.x.z
            auto noPrefix=versionLocal_.substr(1);
            if (AscendingOrder()(noPrefix, key)&& IsSameRegKey(noPrefix, key))
            {
                    keys.push_back(iter.key());
            }
        }
        //������ˣ�����Ҫ���Ǹ��ϸ��ˡ����������������ٹ���һ��
        //����ֻ�������µİ�
        std::sort(keys.begin(), keys.end(), AscendingOrder());
        if (keys.size() > 1)
        {
            keys.erase(keys.begin(), --keys.end());
        }
        return ;
    }
    else
    {
        return ;
    }

}

void QFrameLessWidget_Alime::ReadUpdatePacksInfo()
{
    //
    const nlohmann::json& json = json_["UpdatePacks"];

    bool addImage = false;
    std::string webUrl;
    GetUpdatePackUrl(json, webUrl, addImage);
    if (addImage)
    {
        ReadInstallationCDInfo(pkgList_);
        return;
    }
    else
    {
        QNetworkAccessManager manager;
        {
            QUrl url = ConfigFileReadWriter::Instance().GetUrlOfUpdatePackFolder() + webUrl.c_str();
            QEventLoop loop;
            QNetworkReply* reply = manager.head(QNetworkRequest(url));
            //����������ǰ����, ��ʼ��stackWidget����
            connect(reply, SIGNAL(finished()), &loop, SLOT(quit()), Qt::DirectConnection);
            loop.exec();
            QVariant var = reply->header(QNetworkRequest::ContentLengthHeader);
            if (reply->error() != QNetworkReply::NoError)
            {
                Alime::Console::WriteLine(L"reply error", Alime::Console::RED);
                qDebug() << reply->errorString();
            }
            reply->deleteLater();

            auto pkgSize = var.toLongLong();
            QListWidgetItem* item = new QListWidgetItem();
            QSize preferSize = item->sizeHint();
            item->setSizeHint(QSize(preferSize.width(), 70));
            pkgList_->addItem(item);

            auto itemWidget = new DownloadInfoWidget(this, GetFilePart(url), pkgSize, url);
            pkgList_->setItemWidget(item, itemWidget);

            connect(itemWidget, &DownloadInfoWidget::finishSetup, [&](bool isUpdatePack) {
                if (!itemWidget)
                {
                    int x = 3;//fuck
                }
                if (isUpdatePack)
                {
                    pkgList_->clear();
                    ReadLocalVersion();
                    ReadUpdatePacksInfo();
                    ReadFixPacksInfo();//fix me
                    //����ط����ź��ѿ�~��ȻQtӦ��Ҫ������wpfһ����updatePkgList_=null, updatePkgList_=...
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
