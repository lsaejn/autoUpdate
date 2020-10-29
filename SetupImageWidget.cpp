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
一键更新按钮是意料之外的需求，我们靠奇技淫巧搞定它
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
    isAutoSetupRunning_ = true;
    for(int i=0; i!= array.size(); ++i)
    {
        auto elem = array[i];
        if (!elem)
            continue;
        //没有时间写状态判断了，先暂定下载再说
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