#include "QFrameLessWidget_Alime.h"
#include <QLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QProgressBar>
#include<QMessageBox>

CLASSREGISTER(QFrameLessWidget_Alime)


//我们不使用grid，以便做精细布局
DownloadInfoWidget::DownloadInfoWidget(QWidget* parent)
    :QWidget(parent)
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->addSpacing(5);
    //setAttribute(Qt::WA_StyledBackground, true);
    //debug
    //setStyleSheet("border:2px solid #014F84; background-color:rgb(255,0,0)");
    //压缩图标
    {
        QLabel* iconLabel = new QLabel(this);
        iconLabel->setObjectName("itemIconLabel");
        //QPixmap pixmap = QPixmap::fromImage(QImage(":/images/zip.png")); //新建一个image对象
        //QSize sss = iconLabel->size();
        //QPixmap fitpixmap = pixmap.scaled(sss.width(), sss.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        //iconLabel->setPixmap(fitpixmap); //将图片放入label，使用setPixmap,注意指针*img
        mainLayout->addWidget(iconLabel);
    }
    //文件名，文件大小
    {
        QWidget* fileInfo = new QWidget(this);
        
        QVBoxLayout* fileInfoLayout = new QVBoxLayout();
        fileInfoLayout->setSpacing(0);
//debug
        static int num = 0;
        std::string str = "fuck";
        for (int i = 0; i != num; ++i)
            str += str;
        num++;
        QLabel* fileName = new QLabel(("PkpmV5.1.1-package" + str).c_str());
//
        //fileName->set
        QLabel* fileDownloadHeadway = new QLabel("2.25MB/44.78MB");
        fileDownloadHeadway->setObjectName("grayLabel");
        fileInfoLayout->addWidget(fileName);
        fileInfoLayout->addWidget(fileDownloadHeadway);
        fileInfo->setLayout(fileInfoLayout);
        mainLayout->addWidget(fileInfo);
    }

    mainLayout->addStretch(1);

    //时间估计
    {
        QLabel* leftTimeEstimated = new QLabel(u8"剩余00:10:33");
        leftTimeEstimated->setObjectName("grayLabel");
        leftTimeEstimated->setFixedWidth(100);
        mainLayout->addWidget(leftTimeEstimated);
        mainLayout->addSpacing(10);
    }

    //下载进度+下载速度/状态
    {
        QWidget* stateBox = new QWidget(this);
        QVBoxLayout* downloadStateBox = new QVBoxLayout();
        QProgressBar* bar = new QProgressBar(this);
        downloadStateBox->addWidget(bar);
        bar->setFixedWidth(240);
        QLabel* state = new QLabel(u8"暂停中....");
        state->setObjectName("grayLabel");
        downloadStateBox->addWidget(state);
        stateBox->setLayout(downloadStateBox);
        mainLayout->addWidget(stateBox);
    }
    mainLayout->addSpacing(20);

    {
        QPushButton* downloadSwitch = new QPushButton(this);
        downloadSwitch->setObjectName("ItemPlay");
        downloadSwitch->setToolTip(u8"开始");

        QPushButton* deleteLocalFile = new QPushButton(this);
        deleteLocalFile->setObjectName("ItemDelete");
        deleteLocalFile->setToolTip(u8"取消下载");

        QPushButton* openFolder = new QPushButton(this);
        openFolder->setObjectName("ItemOpenFolde");
        openFolder->setToolTip(u8"打开所在文件夹");

        mainLayout->addWidget(downloadSwitch);
        mainLayout->addSpacing(15);
        mainLayout->addWidget(deleteLocalFile);
        mainLayout->addSpacing(15);
        mainLayout->addWidget(openFolder);
        mainLayout->addSpacing(15);
    }


}

DownloadInfoWidget::~DownloadInfoWidget()
{

}



QFrameLessWidget_Alime::QFrameLessWidget_Alime(QWidget *parent)
    : Alime_ContentWidget(parent)
{
    QHBoxLayout* contentLayout = new QHBoxLayout(this);
    leftContent_ = new QWidget(this);
    rightContent_ = new QWidget(this);
    contentLayout->addWidget(leftContent_);
    contentLayout->addWidget(rightContent_);
    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);
    contentLayout->setStretch(0, 1);
    contentLayout->setStretch(1, 4);

    leftContent_->setObjectName("leftContent");
    //leftContent->setFixedWidth(240);

    rightContent_->setObjectName("rightContent");


    QVBoxLayout* leftLayout = new QVBoxLayout(leftContent_);
    leftLayout->setMargin(0);


    QPushButton* btn01 = new QPushButton(u8"补丁包");
    btn01->setObjectName("btnBoard");
    btn01->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    leftLayout->addWidget(btn01);

    QPushButton* btn02 = new QPushButton(u8"光盘");
    btn02->setObjectName("btnBoard");
    leftLayout->addWidget(btn02);
    btn02->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QPushButton* btn03= new QPushButton("click me");
    btn03->setObjectName("btnBoard");
    btn03->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    leftLayout->addWidget(btn03);

    QPushButton* btn04 = new QPushButton("click me");
    btn04->setObjectName("btnBoard");
    btn04->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    leftLayout->addWidget(btn04);

    InitDownloadList();
}

bool QFrameLessWidget_Alime::InitDownloadList()
{

    QVBoxLayout* vbox = new QVBoxLayout(rightContent_);
    {
        downloadList_ = new QListWidget(this);
        vbox->addWidget(downloadList_);

        for (int i = 0; i != 3; ++i)
        {
            QListWidgetItem* item = new QListWidgetItem();
            QSize size = item->sizeHint();
            item->setSizeHint(QSize(size.width(), 70));
            downloadList_->addItem(item);
            auto fuckWid = new DownloadInfoWidget(this);
            downloadList_->setItemWidget(item, fuckWid);
        }
    }
    return true;
}
