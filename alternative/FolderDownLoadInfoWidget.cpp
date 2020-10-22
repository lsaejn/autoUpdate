#include <QLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QMessageBox>
#include <QUrl>
#include <QApplication>
#include <QStackedWidget>
#include <QButtonGroup>

#include "FolderDownloadInfoWidget.h"

ComparisonDownloadInfoWidget::ComparisonDownloadInfoWidget
	(QWidget* parent, const QString& version)
    :version_(version)
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->addSpacing(5);
    mainLayout->setSpacing(0);
    //versionNameLabel_

    QLabel* iconLabel = new QLabel(this);
    iconLabel->setObjectName("folderIconLabel");
    mainLayout->addWidget(iconLabel);
    mainLayout->addSpacing(5);

    versionNameLabel_ = new QLabel(version, this);
    versionNameLabel_->setObjectName("filePackLabel");

    mainLayout->addWidget(versionNameLabel_);
    mainLayout->addSpacing(5);
    //QWidget* fileInfoWidget = new QWidget(this);
    //QVBoxLayout* vBox = new QVBoxLayout(fileInfoWidget);
    
    fileDownloadingLabel_ = new  QLabel(u8"正在下载", this);
    mainLayout->addWidget(fileDownloadingLabel_);
    mainLayout->addSpacing(5);
    fileDownloadingLabel_->setStyleSheet("border-color: rgb(170, 255, 127);background-color: rgb(0, 255, 127);");

    speed_ = new  QLabel("0Kb/s", this);
    mainLayout->addWidget(speed_);
    mainLayout->addSpacing(5);
    speed_->setObjectName("speedLabel");
    speed_->setAlignment(Qt::AlignCenter);
    speed_->setStyleSheet("border-color: rgb(170, 255, 127);background-color: rgb(0, 255, 0);");
    //mainLayout->addWidget(fileInfoWidget);

    progressBar_ = new QProgressBar(this);
    progressBar_->setFixedWidth(240);
    progressBar_->setFixedHeight(5);
    progressBar_->setRange(0, 100);
    progressBar_->setStyleSheet("QProgressBar{ color:red；background:yellow }");
    mainLayout->addWidget(progressBar_);

    startButton_ =new QPushButton(this);
    startButton_->setObjectName("ItemPlay");
    startButton_->setToolTip(u8"开始");

    connect(startButton_, &QPushButton::clicked, [=]() {
        const QString str = "fuck what happened? lost your fucking mind";
        const QString str2 = "test";
        
        if(fileDownloadingLabel_->text()== str)
            fileDownloadingLabel_->setText(str2);
        else
            fileDownloadingLabel_->setText(str);
        });

    cancelButton_= new QPushButton(this);
    cancelButton_->setObjectName("ItemDelete");
    cancelButton_->setToolTip(u8"取消");
    
    mainLayout->addSpacing(15);
    mainLayout->addWidget(startButton_);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(cancelButton_);
    mainLayout->addSpacing(15);

}