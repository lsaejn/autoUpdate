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
    for(auto elem: array)
    {
        //没有时间判断了，先暂定下载再说
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
                ShowWarningBox(u8"错误", u8"网络中断, 更新失败", u8"确定");
                loop.quit();
                });
            elem->StartDownloadTask();
            loop.exec();
            if (!downloadFinished)//安装过程中，用户关闭窗口
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