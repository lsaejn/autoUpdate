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
/*
һ�����°�ť������֮����������ǿ��漼���ɸ㶨��
*/
void SetupWidget::SetupAllTask()
{
    ALIME_SCOPE_EXIT{
        isAutoSetupRunning_ = false;
    };
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
    for(int i=0; i!= elemNum; ++i)
    {
        auto elem = array[i];
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
                downloadFinished = true;
                ShowWarningBox(u8"����", u8"�����ж�, ����ʧ��", u8"ȷ��");
                loop.quit();
                });
            emit installing(i+1);
            elem->StartDownloadTask();
            loop.exec();
            if (!downloadFinished)//��װ�����У��û��رմ���
                return;//disconnect here
        }

        QEventLoop loopSetup;
        std::atomic<bool> setupFinished = false;
        connect(elem, &DownloadInfoWidget::finishSetup, [&](){
                setupFinished = true;
                loopSetup.quit();
            });
        elem->DoSetup();
        if (!setupFinished)
            loopSetup.exec();
        //emit finish(i);//?
        elem->disconnect(this);
    }
    emit finish(2);//fix me, new signal replace this
    disconnect();
}

bool SetupWidget::IsAutoSetupOn()
{
    return isAutoSetupRunning_;
}