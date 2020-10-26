#include "SetupImageWidget.h"
#include "DownloadInfoWidget.h"
#include <QLayout>
#include <QLabel>
#include <QEventLoop>

//#include <QArray>

SetupWidget::SetupWidget(QWidget* parent)
	:QListWidget(parent),
    isAutoSetupRunning_(false)
{
	setObjectName("SetupImageWidget");
}

#include "Alime/ScopeGuard.h"
void SetupWidget::SetupAllTask()
{
    isAutoSetupRunning_ = true;
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
    for(auto elem: array)
    {
        if (!elem->IsFinished())
        {
            QEventLoop loop;
            std::atomic<bool> downloadFailed = false;
            //std::atomic<bool> downloadFinished = false;// optimize
            connect(elem, &DownloadInfoWidget::finishDownload, &loop, &QEventLoop::quit, Qt::DirectConnection);
            connect(elem, &DownloadInfoWidget::errorDownload, &loop, [&]() {
                downloadFailed = true;
                });
            connect(elem, &DownloadInfoWidget::errorDownload, &loop, &QEventLoop::quit, Qt::DirectConnection);
            elem->StartDownloadTask();

            loop.exec();
            if (downloadFailed)
                return;
        }
        QEventLoop loopSetup;
        std::atomic<bool> setupFinished = false;
        connect(elem, &DownloadInfoWidget::finishSetup, &loopSetup, &QEventLoop::quit, Qt::DirectConnection);
        connect(elem, &DownloadInfoWidget::finishSetup, [&]()
            {
                setupFinished = true;
            });
        elem->DoSetup();
        if (!setupFinished)
            loopSetup.exec();
    }
}

bool SetupWidget::IsAutoSetupOn()
{
    return isAutoSetupRunning_;
}