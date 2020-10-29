#include "SetupImageWidget.h"
#include "DownloadInfoWidget.h"
#include <QLayout>
#include <QLabel>
#include <QEventLoop>
#include <QApplication>
#include "AppUtility.h"

//#include <QArray>

SetupWidget::SetupWidget(QWidget* parent)
	:QListWidget(parent),
    isAutoSetupRunning_(false)
{
	setObjectName("SetupImageWidget");
}

#include "Alime/ScopeGuard.h"
#include "TaskThread.h"
/*
һ�����°�ť������֮����������ǿ��漼���ɸ㶨��
*/
void SetupWidget::SetupAllTask()
{
    ALIME_SCOPE_EXIT{
        isAutoSetupRunning_ = false;
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
    isAutoSetupRunning_ = true;
    for(int i=0; i!= array.size(); ++i)
    {
        auto elem = array[i];
        if (!elem)
            continue;
        //û��ʱ��д״̬�ж��ˣ����ݶ�������˵
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
        elem->DoSetup();
        if (!setupFinished)
            loopSetup.exec();
        //emit finish(i);//?
        elem->disconnect(this);
    }
}

bool SetupWidget::IsAutoSetupOn()
{
    return isAutoSetupRunning_;
}

bool SetupWidget::HasSetupItem()
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