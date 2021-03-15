#include "PackageListWidget.h"
#include "DownloadInfoWidget.h"
#include <QLayout>
#include <QLabel>
#include <QEventLoop>
#include <QApplication>
#include "AppUtility.h"
#include "Alime/ScopeGuard.h"
#include "TaskThread.h"

//#include <QArray>

PackageListWidget::PackageListWidget(QWidget* parent)
	:QListWidget(parent)
{
    setObjectName("SetupImageWidget");
}

void PackageListWidget::ReadUpdatePack()
{

}

void PackageListWidget::ReadFixPack()
{

}

void PackageListWidget::ReadSetupImage()
{

}


/*
һ�����°�ť������֮����������ǿ��漼���ɸ㶨��
*/
void PackageListWidget::SetupAllTask()
{
    ALIME_SCOPE_EXIT{
        emit finish(2);//fix me, new signal replace this
        disconnect();
    };
    if (SetupThread::HasInstance())
    {
        ShowWarningBox("error", u8"��ȴ���һ����װִ�����", u8"ȷ��");
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
        //û��ʱ��д״̬�ж��ˣ�����ͣ������˵
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
                ShowWarningBox(u8"����", u8"�����ж�, ����ʧ��", u8"ȷ��");
                loop.quit();
                });
            emit installing(i+1);
            elem->StartDownloadTask();
            loop.exec();
            if (!downloadFinished)//�û��رմ��ڻᵼ��loop����
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
            //�����İ�װû�п�ʼ��iso�ļ�/�û�ȡ��
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