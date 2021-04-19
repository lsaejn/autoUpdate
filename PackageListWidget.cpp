#include "PackageListWidget.h"
#include "DownloadInfoWidget.h"
#include <QLayout>
#include <QLabel>
#include <QEventLoop>
#include <QApplication>
#include "AppUtility.h"
#include "Alime/ScopeGuard.h"
#include "Alime/Console.h"

#include "QFrameLessWidget_Alime.h"

#include "VersionFileFinder.h"
#include "TaskThread.h"

//#include <QArray>

PackageListWidget::PackageListWidget(QWidget* parent)
	:QListWidget(parent)
{
    setObjectName("SetupImageWidget");
}

void PackageListWidget::Parse(const nlohmann::json& json_)
{
    ReadUpdatePack(json_);
    ReadFixPack(json_);
}


void PackageListWidget::GetUpdatePackUrl(const nlohmann::json& json,
    std::string& urlOut, std::string& instructionUrl, bool& showImage)
{
    std::string ver = json["version"];
    std::string url = json["url"];
    std::string insUrl = json["update_description"];
    //��reg����ΪV6���ܲ���Ҫע����൱��һ��bool
    bool hasRegStr = false;
    std::string regKey = (hasRegStr = json.contains("reg")) ? json["reg"] : "";

    //V5֮ǰ �� ֮��Ĵ�汾���ܻ��Ʋ�ͬ��f...u...c...k
    //����ôԤ��δ��?

    if (mainVersionLocal_.find("V5") != std::string::npos)//��ΪV5֮ǰû�Զ�����
    {
        //ע���ͬ��ֱ�Ӹ�����
        if (mainVersionLocal_ != regKey)
        {
            showImage = true;
            return;
        }
        else
        {
            Alime::Console::WriteLine(L"ע�����ͬ��Ѱ��������");
            auto ret = AscendingOrder().Compare(versionLocal_, ver);
            if (ret >= 0)
            {
                Alime::Console::WriteLine(L"�������°汾");
                return;
            }
            else
            {
                urlOut = url;
                instructionUrl = insUrl;
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
            auto noPrefix = versionLocal_.substr(1);
            if (AscendingOrder()(noPrefix, key) && IsSameRegKey(noPrefix, key))
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
        return;
    }
    else
    {
        return;
    }

}


void PackageListWidget::ReadUpdatePack(const nlohmann::json& json_)
{
    const nlohmann::json& json = json_["UpdatePacks"];

    bool addImage = false;
    std::string instructionUrl;
    std::string webUrl;
    GetUpdatePackUrl(json, webUrl, instructionUrl, addImage);
    if (addImage)
    {
        ReadSetupImage(json_);
        return;
    }
    else
    {
        if (webUrl.empty())
            return;
        QNetworkAccessManager manager;
        {
            QUrl url = ConfigFileReadWriter::Instance().GetUrlOfUpdatePackFolder() + webUrl.c_str();
            QUrl insUrl(instructionUrl.c_str());
            QEventLoop loop;
            QNetworkReply* reply = manager.head(QNetworkRequest(url));
            //����������ǰ����, ��ʼ��stackWidget����
            connect(reply, SIGNAL(finished()), &loop, SLOT(quit()), Qt::DirectConnection);
            loop.exec();
            QVariant var = reply->header(QNetworkRequest::ContentLengthHeader);
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError)
            {
                Alime::Console::WriteLine(L"reply error", Alime::Console::RED);
                qDebug() << reply->errorString();
                return;
            }
            auto pkgSize = var.toLongLong();
            auto item = AddItem(this, pkgSize, url, insUrl, GetFilePart(url), PackType::UpdatePack);

            connect(item, &DownloadInfoWidget::finishSetup, this, &PackageListWidget::ReReadPacks);
        }
    }
}

bool IsFileExistInCfgFolder(const std::string& filename)
{
    auto str = GetApplicationDirPath() + "../CFG/" + filename.c_str();
    QFile f(str);
    return f.exists();
}

void PackageListWidget::ReadFixPack(const nlohmann::json& json_)
{
    auto debug = json_.dump(4);
    const nlohmann::json& json = json_["FixPacks"];
    std::string version = versionLocal_;
    if (!version.empty() && json.find(version) != json.end())
    {
        // not an array in fact
        nlohmann::json array = json_["FixPacks"][version];
        auto sz = array.size();
        QNetworkAccessManager manager;
        if (true)
        {
            if (IsFileExistInCfgFolder(array["patchName"]))
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
            auto item = AddItem(this, pkgSize, url, QUrl(array["update_description"].get<std::string>().c_str()), GetFilePart(qUrl), PackType::FixPack);

            connect(item, &DownloadInfoWidget::finishSetup, this, &PackageListWidget::ReReadPacks);
        }
    }
}

void PackageListWidget::ReadSetupImage(const nlohmann::json& json_)
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
    auto item=AddItem(this, pkgSize, url, QUrl(json_["update_description"].get<std::string>().c_str()), GetFilePart(url), PackType::Image);
    connect(item, &DownloadInfoWidget::finishSetup, this, &PackageListWidget::ReReadPacks);
}

bool PackageListWidget::HasSetupItem()
{
    int elemNum = count();
    for (int i = 0; i != elemNum; ++i)
    {
        auto elem = item(i);
        DownloadInfoWidget* item = dynamic_cast<DownloadInfoWidget*>(itemWidget(elem));
        if (item && (item->IsSetuping() || item->IsDownLoading()))
        {
            return true;
        }
    }
    return false;
}

DownloadInfoWidget* PackageListWidget::AddItem(QWidget* _parent,
    qint64 _fileSize, const QUrl& _url, const QUrl& ins_url, const QString& filename, PackType ty)
{
    QListWidgetItem* item = new QListWidgetItem();
    QSize preferSize = item->sizeHint();
    item->setSizeHint(QSize(400, 80));
    addItem(item);
    auto itemWidget = new DownloadInfoWidget(this, filename, _fileSize, _url, ins_url, ty);
    setItemWidget(item, itemWidget);
    return itemWidget;
}

void PackageListWidget::SetVersion(const std::string& mainV, const std::string& localV)
{
    mainVersionLocal_ = mainV;
    versionLocal_ = localV;
}

void PackageListWidget::ReReadPacks()
{
    for (int i = 0; i != this->count(); ++i)
    {
        auto elem = this->item(i);
        elem->setHidden(true);
    }
    this->clear();

    //delete these
    {
        auto c = this->count();
        assert(c == 0);
        Alime::Console::WriteLine(L"cleared");
    }

    mainWidget_->InitDownloadList();
}