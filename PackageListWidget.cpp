#include "PackageListWidget.h"
#include "DownloadInfoWidget.h"
#include <QLayout>
#include <QLabel>
#include <QEventLoop>
#include <QApplication>
#include "AppUtility.h"
#include "Alime/ScopeGuard.h"
#include "Alime/Console.h"

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
    std::string& urlOut, bool& showImage)
{
    std::string ver = json["version"];
    std::string url = json["url"];
    //给reg是因为V6可能不需要注册表，相当于一个bool
    bool hasRegStr = false;
    std::string regKey = (hasRegStr = json.contains("reg")) ? json["reg"] : "";

    //V5之前 和 之后的大版本可能机制不同。f...u...c...k
    //我怎么预测未来?

    if (mainVersionLocal_.find("V5") != std::string::npos)//因为V5之前没自动更新
    {
        //注册表不同，直接给光盘
        if (mainVersionLocal_ != regKey)
        {
            showImage = true;
            return;
        }
        else
        {
            Alime::Console::WriteLine(L"注册表不同");
            auto ret = AscendingOrder().Compare(versionLocal_, ver);
            if (ret >= 0)
            {
                Alime::Console::WriteLine(L"已是最新版本");
                return;
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
            //秉承某领导“错了也要能正常工作”的原则
            if (string_utility::startsWith(key.c_str(), "V"))
            {
                key = key.substr(1);
            }
            //这个和业务相关，我们暂时只支持相同注册表的包,也就是V5.x.y只能升级到V5.x.z
            auto noPrefix = versionLocal_.substr(1);
            if (AscendingOrder()(noPrefix, key) && IsSameRegKey(noPrefix, key))
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
    std::string webUrl;
    GetUpdatePackUrl(json, webUrl, addImage);
    if (addImage)
    {
        ReadSetupImage(json_);
        return;
    }
    else
    {
        QNetworkAccessManager manager;
        {
            QUrl url = ConfigFileReadWriter::Instance().GetUrlOfUpdatePackFolder() + webUrl.c_str();
            QEventLoop loop;
            QNetworkReply* reply = manager.head(QNetworkRequest(url));
            //我们阻塞当前代码, 初始化stackWidget内容
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
            addItem(item);

            auto itemWidget = new DownloadInfoWidget(this, GetFilePart(url), pkgSize, url);
            setItemWidget(item, itemWidget);

            connect(itemWidget, &DownloadInfoWidget::finishSetup, [=](bool isUpdatePack) {
                if (!itemWidget)
                {
                    int x = 3;//fuck
                }
                if (isUpdatePack)
                {
                    clear();
                    //Parse(json_);
                    //ReadLocalVersion();
                    //ReadUpdatePacksInfo();
                    //ReadFixPacksInfo();//fix me
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
        nlohmann::json array = json_["FixPacks"][version];
        auto sz = array.size();//sz must equals 1, 由于逻辑大改
        QNetworkAccessManager manager;//网上的意思是最多5个请求
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
            //auto fullName = qUrl.toString();
            //auto fileApart = GetFilePart(fullName);

            auto item = AddItem(this, pkgSize, url, GetFilePart(qUrl));
            connect(item, &DownloadInfoWidget::finishSetup, [this](bool isUpdatePack) {
                for (int i = 0; i != this->count(); ++i)
                {
                    auto elem=this->item(i);
                    elem->setHidden(true);
                }
                this->clear();
                auto c=this->count();
                Alime::Console::WriteLine(L"cleared");
                });
        }
    }
}

void PackageListWidget::ReadSetupImage(const nlohmann::json& json_)
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
    AddItem(this, pkgSize, url, GetFilePart(url));
}

/*
一键更新按钮是意料之外的需求，我们靠奇技淫巧搞定它
*/
void PackageListWidget::SetupAllTask()
{
    ALIME_SCOPE_EXIT{
        emit finish(2);//fix me, new signal replace this
        disconnect();
    };
    if (SetupThread::HasInstance())
    {
        ShowWarningBox("error", u8"请等待另一个安装执行完成", u8"确定");
        return;
    }

    int elemNum=count();
    QVector<DownloadInfoWidget*> array{nullptr, nullptr };
    for (int i = 0; i != elemNum; ++i)
    {
        auto elem = item(i);
        DownloadInfoWidget* widget = dynamic_cast<DownloadInfoWidget*>(itemWidget(elem));
        if (widget)
        {
            if (widget->IsUpdatePackage())
                array[0] = widget;
            else
                array[1] = widget;
        }
    }
    //fuuuuuuuuuuuuuuuuuuuuuuck
    for(int i=0; i!= array.size(); ++i)
    {
        auto elem = array[i];
        if (!elem)
            continue;
        //没有时间写状态判断了，先暂停下载再说
        while (elem->IsDownLoading())
        {
            elem->PauseDownloadTask();
            qApp->processEvents();
        }

        if (!elem->IsFinished())
        {
            QEventLoop loop;
            std::atomic<bool> downloadFinished = false;
            //std::atomic<bool> downloadFinished = false;// optimize
            connect(elem, &DownloadInfoWidget::finishDownload, [&]() {
                downloadFinished = true;
                loop.quit();
                });
            bool ret=connect(elem, &DownloadInfoWidget::errorDownload, [&]() {
                downloadFinished = false;
                emit error();
                ShowWarningBox(u8"错误", u8"网络中断, 更新失败", u8"确定");
                loop.quit();
                });
            emit installing(i+1);
            elem->StartDownloadTask();
            loop.exec();
            if (!downloadFinished)//用户关闭窗口会导致loop结束
                return;//disconnect here
        }

        QEventLoop loopSetup;
        std::atomic<bool> setupFinished = false;
        connect(elem, &DownloadInfoWidget::finishSetup, [&](){
                setupFinished = true;
                loopSetup.quit();
            });
        emit installing(i + 1);
        bool succ=elem->DoSetup();
        if (!succ)
        {
            //真正的安装没有开始，iso文件/用户取消
            setupFinished = true;
        }
        if (!setupFinished)
            loopSetup.exec();
        //emit finish(i);//?
        //elem->disconnect(this);
    }
}

//bool PackageListWidget::IsAutoSetupOn()
//{
//    return isAutoSetupRunning_;
//}

bool PackageListWidget::HasSetupItem()
{
    int elemNum = count();
    for (int i = 0; i != elemNum; ++i)
    {
        auto elem = item(i);
        DownloadInfoWidget* item = dynamic_cast<DownloadInfoWidget*>(itemWidget(elem));
        if (item && item->IsSetuping())
        {
            return true;
        }
    }
    return false;
}

DownloadInfoWidget* PackageListWidget::AddItem(QWidget* _parent,
    qint64 _fileSize, const QUrl& _url, const QString& filename)
{
    QListWidgetItem* item = new QListWidgetItem();
    QSize preferSize = item->sizeHint();
    item->setSizeHint(QSize(preferSize.width(), 70));
    addItem(item);
    //auto index = _url.lastIndexOf("/");
    auto itemWidget = new DownloadInfoWidget(this, filename, _fileSize, _url);
    setItemWidget(item, itemWidget);
    return itemWidget;
}

void PackageListWidget::SetVersion(const std::string& mainV, const std::string& localV)
{
    mainVersionLocal_ = mainV;
    versionLocal_ = localV;
}