#include "DownloadInfoWidget.h"
#include "Alime/ScopeGuard.h"
#include "Alime/time/Timestamp.h"
#include "Alime/time/Duration.h"
#include "AppUtility.h"

#include <QLayout>
#include <QPushbutton>
#include <QLabel>
#include <QProgressBar>
#include <QMessageBox>
#include <QDir>
#include <QMenu>


//我们不使用grid，以便做精细布局
DownloadInfoWidget::DownloadInfoWidget(QWidget* _parent, const QString& _fileName, uint64_t _fileSize, const QString& _url)
    :QWidget(_parent),
    url_(_url),
    fileDownloadHeadway_(nullptr),
    leftTimeEstimated_(nullptr),
    downloadStatusLabel_(nullptr),
    progressBar_(nullptr),
    downloadButton_(nullptr),
    bytesDown_ (0),
    totalSize_(_fileSize),
    downloadState_(DownloadState::NotStarted),
    fileName_ (_fileName),
    reply_(nullptr)
{
    //fix me, downloadDirectory测试后删除
    localFilePath_ = GetDownloadFolder()+fileName_;
    setContextMenuPolicy(Qt::CustomContextMenu);
    QMenu* lableMenu = new QMenu(this);

    lableMenu->addAction(QIcon(":/images/play.png"), u8"开始");
    lableMenu->addSeparator();
    lableMenu->addAction(QIcon(":/images/pause.png"), u8"暂停");
    lableMenu->addSeparator();
    lableMenu->addAction(QIcon(":/images/close-gray.png"), u8"删除");
    connect(this, &QWidget::customContextMenuRequested, [=](const QPoint& pos){
            lableMenu->exec(QCursor::pos());
        });
    connect(lableMenu, &QMenu::triggered, [=](QAction* action) {
        QString str=action->text();
        if (str == u8"开始") 
            StartDownloadTask();
        else if (str == u8"暂停") 
            PauseDownloadTask();
        else 
            CancelDownloadTask();
        });

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->addSpacing(5);
    mainLayout->setSpacing(0);

    //debug
    setAttribute(Qt::WA_StyledBackground, true);
    //setStyleSheet("border:2px solid #014F84; background-color:rgb(255,0,0)");

    ///图标
    {
        QLabel* iconLabel = new QLabel(this);
        iconLabel->setObjectName("itemIconLabel");
        mainLayout->addWidget(iconLabel);
    }
    ///文件名，文件大小
    {
        QWidget* fileInfo = new QWidget(this);
        fileInfo->setMinimumWidth(400);
        mainLayout->addWidget(fileInfo);

        QVBoxLayout* fileInfoLayout = new QVBoxLayout();
        fileInfoLayout->setSpacing(0);
        fileInfo->setLayout(fileInfoLayout);

        fileNameLabel_ = new QLabel(_fileName);
        fileInfoLayout->addWidget(fileNameLabel_);

        fileDownloadHeadway_ = new QLabel(MakeDownloadHeadway(0, _fileSize));
        fileDownloadHeadway_->setObjectName("grayLabel");
        fileInfoLayout->addWidget(fileDownloadHeadway_);
    }

    mainLayout->addStretch(1);

    ///下载进度+下载速度/状态+时间估计
    {
        QWidget* stateBox = new QWidget(this);

        QVBoxLayout* downloadStateBox = new QVBoxLayout(stateBox);
        progressBar_ = new QProgressBar(this);
        progressBar_->setFixedWidth(240);
        progressBar_->setRange(0, 100);
        downloadStateBox->addWidget(progressBar_);

        {
            QWidget* stateSubBox = new QWidget();
            QHBoxLayout* subBox = new QHBoxLayout(stateSubBox);
            subBox->setMargin(0);
            downloadStatusLabel_ = new QLabel(u8"暂停中....");
            downloadStatusLabel_->setObjectName("grayLabel");
            subBox->addWidget(downloadStatusLabel_);

            leftTimeEstimated_ = new QLabel(u8"--");
            leftTimeEstimated_->setObjectName("grayLabel");
            leftTimeEstimated_->setFixedWidth(60);
            subBox->addWidget(leftTimeEstimated_);

            downloadStateBox->addWidget(stateSubBox);
        }

        mainLayout->addWidget(stateBox);
    }
    mainLayout->addSpacing(20);

    if (isBreakPointTranSupported_)
        LoadingProgressForBreakPoint();

    /// 按钮
    {
        downloadButton_ = new QPushButton(this);
        downloadButton_->setObjectName("ItemPlay");
        downloadButton_->setToolTip(u8"开始");
        connect(downloadButton_, &QPushButton::clicked, this, &DownloadInfoWidget::StartDownloadTask);

        pauseButton_= new QPushButton(this);
        pauseButton_->setObjectName("ItemPlayPause");
        pauseButton_->setToolTip(u8"暂停");
        pauseButton_->hide();
        connect(pauseButton_, &QPushButton::clicked, this, &DownloadInfoWidget::PauseDownloadTask);

        QPushButton* deleteLocalFile = new QPushButton(this);
        deleteLocalFile->setObjectName("ItemDelete");
        deleteLocalFile->setToolTip(u8"取消下载");
        connect(deleteLocalFile, &QPushButton::clicked, this,&DownloadInfoWidget::CancelDownloadTask);

        QPushButton* openFolder = new QPushButton(this);
        openFolder->setObjectName("ItemSetup");
        openFolder->setToolTip(u8"开始安装");
        connect(openFolder, &QPushButton::clicked, this, &DownloadInfoWidget::OpenDownloadFolder);

        mainLayout->addWidget(downloadButton_);
        mainLayout->addWidget(pauseButton_);
        mainLayout->addSpacing(15);
        mainLayout->addWidget(deleteLocalFile);
        mainLayout->addSpacing(15);
        mainLayout->addWidget(openFolder);
        mainLayout->addSpacing(15);
    }
}

//辅助函数
//fileNameLabel_ 是控件
std::unique_ptr<QFile> DownloadInfoWidget::openFileForWrite(const QString& filePath)
{
    std::unique_ptr<QFile> file(new QFile(filePath));
    if (!file->open(QIODevice::WriteOnly|QIODevice::Append))
    {
        qCritical() << QString("Unable to open/save the file %1: %2.").
            arg(QDir::toNativeSeparators(filePath),file->errorString());
        return nullptr;
    }
    return file;
}

bool DownloadInfoWidget::StartDownloadTask()
{
    if (url_.isEmpty())
    {
        qCritical() << "empty url";
        return false;//fix me, 提示无法下载
    }  
    //for for file protocol
    const QUrl newUrl = QUrl::fromUserInput(url_);
    if (!newUrl.isValid())
    {
        qCritical() << QString("Invalid URL: %1: %2").arg(url_, newUrl.errorString());
        return false;
    }

    QString fileName = newUrl.fileName();
    if (fileName.isEmpty())
    {
        qCritical() << QString("Invalid URL: %1: %2").arg(url_, newUrl.errorString());
        return false;
    }

    //fix me
    if (QFile::exists(localFilePath_))
    {
        QFileInfo fileInfo(localFilePath_);
        auto fileSize=fileInfo.size();
        if (fileSize == totalSize_)//我不确定是不是要支持断点传输
        {
            if (QMessageBox::question(this, u8"操作警告",
                QString(u8"重新下载该文件？已下载的文件%1将被删除 ." " 确认重下?"). arg(fileName),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No)== QMessageBox::No)
            {
                return false;
            }
            else
            {
                file_.reset();
                QFile::remove(localFilePath_);
                LoadingProgressForBreakPoint();
            }
        }
        else if(fileSize > totalSize_)
        {
            qDebug() << u8"invalid local file size, should not larger than targetSize";
            file_.reset();
            QFile::remove(localFilePath_);
        }
    }

    LoadingProgressForBreakPoint();

    file_ = openFileForWrite(localFilePath_);
    if (!file_)
    {
        QMessageBox::information(this, u8"无法打开文件",
            u8"文件可能被占用，无法写入",
            QMessageBox::Yes);
        return false;
    }
           
    StartRequest(newUrl);
    return true;
}

//用信号是因为下载将来会丢到单独的线程
void DownloadInfoWidget::StartRequest(const QUrl& requestedUrl)
{
    UpdatePlayButton(false);
    QUrl url = requestedUrl;

    QNetworkRequest request(url);
    // 如果支持断点续传，则设置请求头信息
    if (isBreakPointTranSupported_)
    {
        QString strRange = QString("bytes=%1-").arg(bytesDown_);
        request.setRawHeader("Range", strRange.toLatin1());
    }
    reply_ = QNAManager_.get(QNetworkRequest(request));



    connect(reply_, &QNetworkReply::finished, this, &DownloadInfoWidget::httpFinished);
    connect(reply_, &QIODevice::readyRead, this, &DownloadInfoWidget::httpReadyRead);
    connect(reply_, &QNetworkReply::downloadProgress, this, &DownloadInfoWidget::UpdateChildWidgets);
    
    connect(this, &DownloadInfoWidget::notify_progressInfo, progressBar_, &QProgressBar::setValue);
    connect(this, &DownloadInfoWidget::notify_sizeInfo, fileDownloadHeadway_, &QLabel::setText);
    connect(this, &DownloadInfoWidget::notify_stateLabel, downloadStatusLabel_, &QLabel::setText);
    connect(this, &DownloadInfoWidget::notify_timeLabel, leftTimeEstimated_, &QLabel::setText);
}


void DownloadInfoWidget::httpFinished()
{
    file_->flush();
    file_->close();
    file_.reset();
    ALIME_SCOPE_EXIT{
            reply_->deleteLater();
            reply_ = nullptr;
    };

    if (bytesDown_ == totalSize_)
    {
        downloadStatusLabel_->setText(u8"已完成");
        QMessageBox::information(NULL, "Tip", QString(u8"是否立即安装"));
    }
    else if (bytesDown_ < totalSize_ )
    {
        if (reply_->error())
        { 
            if (DownloadState::Paused== downloadState_)
            {
                downloadStatusLabel_->setText(u8"下载暂停");
                qDebug("用户中断/取消了下载");
            }
            else if(DownloadState::Canceled == downloadState_)
            {
                downloadStatusLabel_->setText(u8"下载取消");
                qDebug("用户中断/取消了下载"); 
            }
            else
            {
                downloadState_ = DownloadState::Interrupted;
                downloadStatusLabel_->setText(u8"下载出错");
                qDebug("对端关闭连接/网络中断");
            }
        }
    }
    else
    {
        qDebug("文件大小错误, 可能是上一次没有删除文件");
    }
    leftTimeEstimated_->setText("--");
    UpdatePlayButton();
}

void DownloadInfoWidget::httpReadyRead()
{
    if (file_)
    {
        file_->write(reply_->readAll());
    }
    else
    {
        qWarning() << "ilegal file_ handle";
    }
}

bool DownloadInfoWidget::isTimeToUpdate(double& second)
{
    //我们统一在readCallback里更新，所以可以这么做
    static Alime::Timestamp lastUpdate = Alime::Timestamp::Now();
    Alime::Timestamp now = Alime::Timestamp::Now();
    if (now - lastUpdate >= Alime::Duration::Seconds(1))
    {
        second = (now - lastUpdate).toSeconds();
        lastUpdate = now;
        return true;
    }
    return false;
}

bool DownloadInfoWidget::CancelDownloadTask()
{
    downloadStatusLabel_->setText(u8"下载取消");
    downloadState_ = DownloadState::NotStarted;
    reply_->abort();
    UpdatePlayButton(true);
    return true;
}

bool DownloadInfoWidget::PauseDownloadTask()
{
    downloadStatusLabel_->setText(u8"下载暂停");
    downloadState_ = DownloadState::Paused;
    reply_->abort();
    UpdatePlayButton(true);
    return true;
}

bool DownloadInfoWidget::OpenDownloadFolder()
{
    return true;
}

QString DownloadInfoWidget::MakeDownloadHeadway()
{
    return MakeDownloadHeadway(bytesDown_, totalSize_);
}



QString DownloadInfoWidget::MakeDownloadHeadway(int64_t readed, int64_t total)
{
    QString result = QString::number(ToMByte(readed), 'f', 2);
    result += "MB/";
    result += QString::number(ToMByte(total), 'f', 2) + "MB";
    return result;
}

QString DownloadInfoWidget::MakeDurationToString(int second)
{
    int hours = second / 3600;
    int minutes = (second % 3600) / 60;
    int seconds = second % 60;
    QString timeString = hours > 99 ? "99::99::99":"";
    timeString += QString::number(hours).sprintf("%02d", hours); timeString += ":";
    timeString += QString::number(minutes).sprintf("%02d", minutes); timeString += ":";
    timeString += QString::number(seconds).sprintf("%02d", seconds);
    return timeString;
}

void DownloadInfoWidget::UpdateChildWidgets(qint64 bytesReceived, qint64 bytesTotal)
{
    double secondElepsed = 1.0;
    if (bytesReceived != bytesTotal)
    {
        if (!isTimeToUpdate(secondElepsed))
            return;
    }

    const qint64 size_lastDownloadTask = totalSize_ - bytesTotal;

    double v = (bytesReceived+ size_lastDownloadTask) * 1.0 / (totalSize_) * 100;
    emit notify_progressInfo(v);

    uint64_t increment = bytesReceived+ size_lastDownloadTask - bytesDown_;
    int speed = increment / 1024 / secondElepsed;
    QString result = QString::number(speed);
    result += " KB/s";
    emit notify_stateLabel(result);

    //fix me, 估计剩余时间
    int secondLeftEstimated = (bytesTotal - bytesReceived) / (increment / secondElepsed);
    emit notify_timeLabel(MakeDurationToString(secondLeftEstimated));

    bytesDown_ = bytesReceived + size_lastDownloadTask;
    emit notify_sizeInfo(MakeDownloadHeadway());
}

void DownloadInfoWidget::UpdatePlayButton(bool stopped)
{
    if (stopped)
    {
        downloadButton_->show();
        pauseButton_->hide();
    }
    else
    {
        downloadButton_->hide();
        pauseButton_->show();
    }
}

//重读一次文件
void DownloadInfoWidget::LoadingProgressForBreakPoint()
{
    QFileInfo fileInfo(localFilePath_);
    if (!fileInfo.exists())
    {
        bytesDown_ = 0;
        return;
    }
    else
    {
        bytesDown_=fileInfo.size();
        if (bytesDown_ > totalSize_)
        {
            qWarning() << "bad file size";
            downloadStatusLabel_->setText(u8"文件大小错误，建议重新下载");
            QFile f(localFilePath_);
            f.open(QIODevice::WriteOnly);
            f.remove();
            f.close();
        }
        else if (bytesDown_ == totalSize_)
        {
            downloadStatusLabel_->setText(u8"已完成");
            fileDownloadHeadway_->setText(MakeDownloadHeadway());
            progressBar_->setValue(100);
        }
        else
        {
            fileDownloadHeadway_->setText(MakeDownloadHeadway());
            double v = bytesDown_ * 1.0 / totalSize_ * 100;
            progressBar_->setValue(v);
        }
    }
    //file_.reset()
}