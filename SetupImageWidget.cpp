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
一键更新按钮是意料之外的需求，我们靠奇技淫巧搞定它
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
                downloadFinished = true;
                ShowWarningBox(u8"错误", u8"网络中断, 更新失败", u8"确定");
                loop.quit();
                });
            emit installing(i+1);
            elem->StartDownloadTask();
            loop.exec();
            if (!downloadFinished)//安装过程中，用户关闭窗口
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