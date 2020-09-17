#include "DownloadInfoWidget.h"
#include "Alime/ScopeGuard.h"

#include <QLayout>
#include <QPushbutton>
#include <QLabel>
#include <QProgressBar>
#include <QMessageBox>
#include <QDir>
#include <QMenu>


//���ǲ�ʹ��grid���Ա�����ϸ����
DownloadInfoWidget::DownloadInfoWidget(QWidget* _parent, const QString& _fileName, uint64_t _fileSize, const QString& _url)
    :QWidget(_parent),
    url_(_url),
    fileDownloadHeadway_(nullptr),
    leftTimeEstimated_(nullptr),
    state_(nullptr),
    bar_(nullptr),
    downloadSwitch_(nullptr),
    sizeDown_ (0),
    totalSize_(_fileSize),
    downloadState_(DownloadState::Normal)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    QMenu* lableMenu = new QMenu(this);
    //lableMenu->
    lableMenu->addAction(u8"��ʼ");
    lableMenu->addSeparator();
    lableMenu->addAction(u8"ɾ��");
    lableMenu->addSeparator();
    lableMenu->addAction(u8"��ͣ");
    connect(this, &QWidget::customContextMenuRequested, [=](const QPoint& pos)
        {
            qDebug() << pos;//����pos���������Ҽ����ʱ���������꣬�������һ��������ڿؼ����ϽǶ��Ե�
            lableMenu->exec(QCursor::pos());
        });

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->addSpacing(5);
    mainLayout->setSpacing(0);

    //debug
    setAttribute(Qt::WA_StyledBackground, true);
    //setStyleSheet("border:2px solid #014F84; background-color:rgb(255,0,0)");

    //���ҵ���д
    ///ͼ��
    {
        QLabel* iconLabel = new QLabel(this);
        iconLabel->setObjectName("itemIconLabel");
        mainLayout->addWidget(iconLabel);
    }
    ///�ļ������ļ���С
    {
        QWidget* fileInfo = new QWidget(this);
        fileInfo->setMinimumWidth(400);
        mainLayout->addWidget(fileInfo);

        QVBoxLayout* fileInfoLayout = new QVBoxLayout();
        fileInfoLayout->setSpacing(0);
        fileInfo->setLayout(fileInfoLayout);

        fileName_ = new QLabel(_fileName);
        fileInfoLayout->addWidget(fileName_);

        fileDownloadHeadway_ = new QLabel(MakeDownloadHeadway(0, _fileSize));
        fileDownloadHeadway_->setObjectName("grayLabel");
        fileInfoLayout->addWidget(fileDownloadHeadway_);
    }

    mainLayout->addStretch(1);

    ///���ؽ���+�����ٶ�/״̬+ʱ�����
    {
        QWidget* stateBox = new QWidget(this);

        QVBoxLayout* downloadStateBox = new QVBoxLayout(stateBox);
        bar_ = new QProgressBar(this);
        bar_->setFixedWidth(240);
        bar_->setRange(0, 100);
        downloadStateBox->addWidget(bar_);

        {
            QWidget* stateSubBox = new QWidget();
            QHBoxLayout* subBox = new QHBoxLayout(stateSubBox);
            subBox->setMargin(0);
            state_ = new QLabel(u8"��ͣ��....");
            state_->setObjectName("grayLabel");
            subBox->addWidget(state_);

            leftTimeEstimated_ = new QLabel(u8"--");
            leftTimeEstimated_->setObjectName("grayLabel");
            leftTimeEstimated_->setFixedWidth(60);
            subBox->addWidget(leftTimeEstimated_);

            downloadStateBox->addWidget(stateSubBox);
        }

        mainLayout->addWidget(stateBox);
    }
    mainLayout->addSpacing(20);

    /// ��ť
    {
        downloadSwitch_ = new QPushButton(this);
        downloadSwitch_->setObjectName("ItemPlay");
        downloadSwitch_->setToolTip(u8"��ʼ");
        connect(downloadSwitch_, &QPushButton::clicked, this, &DownloadInfoWidget::StartDownloadTask);

        pauseSwitch_= new QPushButton(this);
        pauseSwitch_->setObjectName("ItemPlayPause");
        pauseSwitch_->setToolTip(u8"��ͣ");
        pauseSwitch_->hide();
        connect(pauseSwitch_, &QPushButton::clicked, this, &DownloadInfoWidget::CancelDownloadTask);

        QPushButton* deleteLocalFile = new QPushButton(this);
        deleteLocalFile->setObjectName("ItemDelete");
        deleteLocalFile->setToolTip(u8"ȡ������");
        connect(deleteLocalFile, &QPushButton::clicked, this,&DownloadInfoWidget::CancelDownloadTask);

        QPushButton* openFolder = new QPushButton(this);
        openFolder->setObjectName("ItemSetup");
        openFolder->setToolTip(u8"��ʼ��װ");
        connect(openFolder, &QPushButton::clicked, this, &DownloadInfoWidget::OpenDownloadFolder);

        mainLayout->addWidget(downloadSwitch_);
        mainLayout->addWidget(pauseSwitch_);
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


std::unique_ptr<QFile> openFileForWrite(const QString& fileName)
{
    std::unique_ptr<QFile> file(new QFile(fileName));
    if (!file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(NULL, "Error", 
            QString("Unable to save the file %1: %2.").arg(QDir::toNativeSeparators(fileName),
                file->errorString()));
        return nullptr;
    }
    return file;
}


bool DownloadInfoWidget::StartDownloadTask()
{
    if (url_.isEmpty())
        return false;//fix me, ��ʾ�޷�����

    const QUrl newUrl = QUrl::fromUserInput(url_);
    if (!newUrl.isValid()) {
        QMessageBox::information(this, tr("Error"),
            tr("Invalid URL: %1: %2").arg(url_, newUrl.errorString()));
        return false;;
    }

    QString fileName = newUrl.fileName();
    if (fileName.isEmpty())
    {
        QMessageBox::information(this, tr("Error"),
            tr("Invalid URL: %1: %2").arg(url_, newUrl.errorString()));
        return false;;
    }
    QString downloadDirectory = QDir::currentPath();
    bool useDirectory = !downloadDirectory.isEmpty() && QFileInfo(downloadDirectory).isDir();
    if (useDirectory)
        fileName.prepend(downloadDirectory + '/');
    if (QFile::exists(fileName)) {
        if (QMessageBox::question(this, tr("Overwrite Existing File"),
            tr("There already exists a file called %1%2."
                " Overwrite?")
            .arg(fileName,
                useDirectory
                ? QString()
                : QStringLiteral(" in the current directory")),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No)
            == QMessageBox::No) {
            return false;
        }
        QFile::remove(fileName);
    }

    file_ = openFileForWrite(fileName);
    if (!file_)
        return false;

    //downloadButton->setEnabled(false);

    // schedule the request
    StartRequest(newUrl);
}

//���ź�����Ϊ���ؽ����ᶪ���������߳�
void DownloadInfoWidget::StartRequest(const QUrl& requestedUrl)
{
    QUrl url = requestedUrl;
    //httpRequestAborted = false;

    reply = qnam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, &DownloadInfoWidget::httpFinished);
    connect(reply, &QIODevice::readyRead, this, &DownloadInfoWidget::httpReadyRead);
    connect(reply, &QNetworkReply::downloadProgress, this, &DownloadInfoWidget::UpdateChildWidgets);
    
    connect(this, &DownloadInfoWidget::notify_progressInfo, bar_, &QProgressBar::setValue);
    connect(this, &DownloadInfoWidget::notify_sizeInfo, fileDownloadHeadway_, &QLabel::setText);
    connect(this, &DownloadInfoWidget::notify_stateLabel, state_, &QLabel::setText);
    connect(this, &DownloadInfoWidget::notify_timeLabel, leftTimeEstimated_, &QLabel::setText);
}


void DownloadInfoWidget::httpFinished()
{
    ALIME_SCOPE_EXIT{
            reply->deleteLater();
            reply = nullptr;
    };
    file_->flush();
    file_->close();
    if (sizeDown_ == totalSize_)
    {
        state_->setText(u8"�����");
        QMessageBox::information(NULL, "Tip", QString(u8"�Ƿ�������װ"));
    }
    else if (sizeDown_ < totalSize_ || reply->error())
    {
        if (reply->error())
        {
            qDebug("�����Ƿ������ض���, ���߶Զ˹ر�����");
        }
        if(!stopped_)
            state_->setText(u8"�����ж�");
    }
    else
    {
        qDebug("�ļ���С����, ��������һ��û��ɾ���ļ�");
    }
    leftTimeEstimated_->setText("--");
    UpdatePlayButton();
}

void DownloadInfoWidget::httpReadyRead()
{
    if (file_)
        file_->write(reply->readAll());
    UpdatePlayButton(false);
}

#include "Alime/time/Timestamp.h"
#include "Alime/time/Duration.h"
bool DownloadInfoWidget::isTimeToUpdate(double& second)
{
    //����ͳһ��readCallback����£����Կ�����ô��
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
    state_->setText(u8"����ȡ��");
    stopped_ = true;
    reply->abort();
    UpdatePlayButton(true);
    return true;
}

bool DownloadInfoWidget::OpenDownloadFolder()
{
    return true;
}

QString DownloadInfoWidget::MakeDownloadHeadway()
{
    return MakeDownloadHeadway(sizeDown_, totalSize_);
}

double ToMByte(int sizeInBit)
{
    return sizeInBit * 1.0 / (1024 * 1024);
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

    double v = bytesReceived * 1.0 / bytesTotal * 100;
    emit notify_progressInfo(v);

    uint64_t increment = bytesReceived - sizeDown_;
    int speed = increment / 1024 / secondElepsed;//kb/s
    QString result = QString::number(speed);
    result += " KB/s";
    emit notify_stateLabel(result);

    //fix me, ����ʣ��ʱ��
    int secondLeftEstimated = (bytesTotal - bytesReceived) / (increment / secondElepsed);
    emit notify_timeLabel(MakeDurationToString(secondLeftEstimated));
    sizeDown_ = bytesReceived;
    totalSize_ = bytesTotal;
    emit notify_sizeInfo(MakeDownloadHeadway());
}

void DownloadInfoWidget::UpdatePlayButton(bool stopped)
{
    if (stopped)
    {
        downloadSwitch_->show();
        pauseSwitch_->hide();
    }
    else
    {
        downloadSwitch_->hide();
        pauseSwitch_->show();
    }
}