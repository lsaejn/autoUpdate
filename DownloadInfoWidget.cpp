#include <QDir>
#include <QLabel>
#include <QMenu>
#include <QLayout>
#include <QPushbutton>
#include <QProgressBar>
#include <QMessageBox>
#include <QProcess>

#include <thread>

#include "DownloadInfoWidget.h"
#include "Alime/ScopeGuard.h"
#include "Alime/time/Timestamp.h"
#include "Alime/time/Duration.h"
#include "AppUtility.h"


//���ǲ�ʹ��grid���Ա�����ϸ����
//��widget��parent������е����⣬����ܵ���һЩ�ڴ�ռ�����⣬�Ҳ�ȷ��
DownloadInfoWidget::DownloadInfoWidget(QWidget* _parent, const QString& _fileName, qint64 _fileSize, const QString& _url)
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
    reply_(nullptr),
    isBreakPointTranSupported_(true)
{
    localFilePath_ = GetDownloadFolder()+fileName_;
    setContextMenuPolicy(Qt::CustomContextMenu);

    //�Ҽ��˵�
    AddMenuItems();

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->addSpacing(5);
    mainLayout->setSpacing(0);

    //debug ������
    setAttribute(Qt::WA_StyledBackground, true);
    //setStyleSheet("border:2px solid #014F84; background-color:rgb(255,0,0)");

    ///ͼ��
    {
        QLabel* iconLabel = new QLabel(this);
        if (_fileName.endsWith("exe"))
        {
            iconLabel->setObjectName("exeIconLabel");
        }
        else if (_fileName.endsWith("iso"))
        {
            iconLabel->setObjectName("isoIconLabel");
        }
        else
        {
            iconLabel->setObjectName("itemIconLabel");
        }
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

        fileNameLabel_ = new QLabel(_fileName);
        fileInfoLayout->addWidget(fileNameLabel_);

        fileDownloadHeadway_ = new QLabel(MakeDownloadHeadway(0, _fileSize));
        fileDownloadHeadway_->setObjectName("grayLabel");
        fileInfoLayout->addWidget(fileDownloadHeadway_);
    }

    mainLayout->addStretch(1);

    ///���ؽ���+�����ٶ�/״̬+ʱ�����
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
            downloadStatusLabel_ = new QLabel(u8"��ͣ��...");
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

    /// ��ť
    {
        downloadButton_ = new QPushButton(this);
        downloadButton_->setObjectName("ItemPlay");
        downloadButton_->setToolTip(u8"��ʼ");
        CHECK_CONNECT_ERROR(connect(downloadButton_, &QPushButton::clicked, this, &DownloadInfoWidget::StartDownloadTask));

        pauseButton_= new QPushButton(this);
        pauseButton_->setObjectName("ItemPlayPause");
        pauseButton_->setToolTip(u8"��ͣ");
        pauseButton_->hide();
        CHECK_CONNECT_ERROR(connect(pauseButton_, &QPushButton::clicked, this, &DownloadInfoWidget::PauseDownloadTask));

        QPushButton* deleteLocalFile = new QPushButton(this);
        deleteLocalFile->setObjectName("ItemDelete");
        deleteLocalFile->setToolTip(u8"ɾ���ļ�");
        CHECK_CONNECT_ERROR(connect(deleteLocalFile, &QPushButton::clicked, this,&DownloadInfoWidget::CancelDownloadTask));

        QPushButton* openFolder = new QPushButton(this);
        openFolder->setObjectName("ItemSetup");
        openFolder->setToolTip(u8"��ʼ��װ");
        CHECK_CONNECT_ERROR(connect(openFolder, &QPushButton::clicked, this, &DownloadInfoWidget::DoSetup));

        mainLayout->addWidget(downloadButton_);
        mainLayout->addWidget(pauseButton_);
        mainLayout->addSpacing(15);
        mainLayout->addWidget(deleteLocalFile);
        mainLayout->addSpacing(15);
        mainLayout->addWidget(openFolder);
        mainLayout->addSpacing(15);
    }
}

//��������
//fileNameLabel_ �ǿؼ�
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
    if(downloadState_!= DownloadState::NotStarted&&
        downloadState_ != DownloadState::Finished)
    {
        qCritical() << u8"״̬����,�������߼�";
    }
    
    if (url_.isEmpty())
    {
        qCritical() << "empty url";
        return false;//fix me, ��ʾ�޷�����
    }  
    //for for file protocol
    const QUrl newUrl = QUrl::fromUserInput(url_);
    if (!newUrl.isValid())
    {
        qCritical() << QString("Invalid URL: %1: %2").arg(url_, newUrl.errorString());
        return false;//ά����Ա�ϴ����ļ�����
    }

    QString fileName = newUrl.fileName();
    if (fileName.isEmpty())
    {
        qCritical() << QString("Invalid URL: %1: %2").arg(url_, newUrl.errorString());
        return false;
    }

    if (QFile::exists(localFilePath_))
    {
        QFileInfo fileInfo(localFilePath_);
        auto fileSize=fileInfo.size();
        if (fileSize == totalSize_)//�Ҳ�ȷ���ǲ���Ҫ֧�ֶϵ㴫��
        {
            auto ret = ShowQuestionBox(u8"��������", 
                QString(u8"�������ظ��ļ��������ص��ļ�%1����ɾ����" " ȷ������?").arg(fileName),
                u8"ȷ��", u8"ȡ��");
            if (!ret)
            {
                return false;
            }
            else
            {
                file_.reset();
                QFile::remove(localFilePath_);
                //LoadingProgressForBreakPoint();
            }
        }
        else if(fileSize > totalSize_)
        {
            qDebug() << u8"Invalid local file size, should not larger than targetSize";
            file_.reset();
            QFile::remove(localFilePath_);
        }
    }

    LoadingProgressForBreakPoint();

    file_ = openFileForWrite(localFilePath_);
    if (!file_)
    {
        ShowWarningBox(u8"�޷����ļ�", u8"�ļ����ܱ�ռ�ã��޷�д��", u8"ȷ��");
        return false;
    }
           
    StartRequest(newUrl);
    downloadState_ = DownloadState::Downloading;
    UpdatePlayButton(false);
    return true;
}


void DownloadInfoWidget::StartRequest(const QUrl& requestedUrl)
{
    QUrl url = requestedUrl;

    QNetworkRequest request(url);
    // ���֧�ֶϵ�����������������ͷ��Ϣ
    if (isBreakPointTranSupported_)
    {
        QString strRange = QString("bytes=%1-").arg(bytesDown_);
        qDebug() << "StartRequest"<<bytesDown_;
        request.setRawHeader("Range", strRange.toLatin1());
    }
    reply_ = QNAManager_.get(QNetworkRequest(request));

    CHECK_CONNECT_ERROR(connect(reply_, &QNetworkReply::finished, this, &DownloadInfoWidget::httpFinished));
    CHECK_CONNECT_ERROR(connect(reply_, &QIODevice::readyRead, this, &DownloadInfoWidget::httpReadyRead));
    CHECK_CONNECT_ERROR(connect(reply_, &QNetworkReply::downloadProgress, this, &DownloadInfoWidget::UpdateChildWidgets));

    //�������ź�����Ϊ���ؽ����ᶪ���������߳�
    CHECK_CONNECT_ERROR(connect(this, &DownloadInfoWidget::notify_progressInfo, progressBar_, &QProgressBar::setValue));
    CHECK_CONNECT_ERROR(connect(this, &DownloadInfoWidget::notify_sizeInfo, fileDownloadHeadway_, &QLabel::setText));
    CHECK_CONNECT_ERROR(connect(this, &DownloadInfoWidget::notify_stateLabel, downloadStatusLabel_, &QLabel::setText));
    CHECK_CONNECT_ERROR(connect(this, &DownloadInfoWidget::notify_timeLabel, leftTimeEstimated_, &QLabel::setText));
}

//�������/��ͣ/ȡ��/����
void DownloadInfoWidget::httpFinished()
{
    ALIME_SCOPE_EXIT{
    reply_->deleteLater();
    reply_ = nullptr;
    };
    {
        file_->flush();
        file_->close();
        file_.reset();
    }

    //��Ӧ�ĳ��ֱ��ض�����������ĳЩ����ϲ�������ֶ��ĵĶ���
    const QVariant redirectionTarget = reply_->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (!redirectionTarget.isNull())
    {
        ShowWarningBox(u8"��ҳ���ض���", u8"�����ڵ����粻֧��Http����", u8"ȷ��");
        const QUrl redirectedUrl = redirectionTarget.toUrl();
        file_ = openFileForWrite(localFilePath_);
        if (!file_)
        {
            qWarning() << "could not open file";
            return;
        }
        StartRequest(redirectedUrl);
        return;
    }

    if (bytesDown_ == totalSize_)
    {
        downloadState_ = DownloadState::Finished;
        downloadStatusLabel_->setText(u8"�����");
        QMessageBox::information(NULL, "Tip", QString(u8"�������"));
    }
    else if (bytesDown_ < totalSize_ )
    {
        if (reply_->error())
        { 
            if (DownloadState::Paused== downloadState_)
            {
                downloadStatusLabel_->setText(u8"��ͣ��...");
                downloadState_ = DownloadState::NotStarted;
                 
                qDebug()<<u8"�û��ж�������  "<< QString::number(bytesDown_);
            }
            else if (DownloadState::Cancel == downloadState_)
            {
                qDebug(u8"�û�ȡ��������");
                downloadStatusLabel_->setText(u8"����ȡ��");
                QFile::remove(localFilePath_);
                LoadingProgressForBreakPoint();
                downloadState_ = DownloadState::NotStarted;
            }
            else if(DownloadState::Downloading == downloadState_)
            {
                downloadState_ = DownloadState::Interrupted;
                downloadStatusLabel_->setText(u8"���س���");
                qDebug(u8"�Զ˹ر�����/�����ж�");
                downloadState_ = DownloadState::Error;
            }
            else
            {
                qDebug(u8"״̬�߼�����,������");
            }
        }
    }
    else
    {
        qDebug(u8"�ļ���С����, �û��ֶ��޸����ļ����ƻ������ص����ݴ���");
        downloadState_ = DownloadState::Error;
        downloadStatusLabel_->setText(u8"��������");
    }
    leftTimeEstimated_->setText("--");
    UpdatePlayButton();
}

void DownloadInfoWidget::httpReadyRead()
{
    if (downloadState_ != DownloadState::Downloading)
    {
        qWarning() << u8"!!!!!!!!fuck what happen";
    }
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
    if (downloadState_ == DownloadState::NotStarted)
    {
        //we do nothing
        QFile::remove(localFilePath_);
        LoadingProgressForBreakPoint();
        return true;
    }
    else if (downloadState_ == DownloadState::Downloading)
    {
        //we updateState in requestFinish as file was writing
        downloadState_ = DownloadState::Cancel;
        reply_->abort();
        return true;
    }
    else if (downloadState_ == DownloadState::Finished)
    {
        QFile::remove(localFilePath_);
        LoadingProgressForBreakPoint();
        downloadState_ = DownloadState::NotStarted;
    }
    else
    {
        qWarning() << "bad logic error in CancelDownloadTask";
    }
    downloadStatusLabel_->setText(u8"��ͣ��...");
    UpdatePlayButton(true);
    return true;
}

bool DownloadInfoWidget::PauseDownloadTask()
{
    if (downloadState_ == DownloadState::Downloading)
    {
        downloadState_ = DownloadState::Paused;
        reply_->abort();
        //fix me, notify UpdatePlayButton by state
        UpdatePlayButton(true);
        return true;
    }
    else
    {
        qWarning() << "bad logic error in PauseDownloadTask";
        return false;
    }
}

#include <QProcess>
#include <QEventLoop>
#include <QThread>
#include "TaskThread.h"
#include "SetupHelper.h"

bool DownloadInfoWidget::DoSetup()
{
    if (SetupThread::HasInstance())
    {
        ShowWarningBox(u8"��������", u8"����ִ����һ����װ", u8"�˳�");
        return false;
    }
        
    ProcessManager checker;
    checker.SetMatchReg(L"PKPMMAIN.EXE");
    checker.ShutDownExistingApp();
    checker.SetMatchReg(L"PKPM[\\d]{4}V[\\d]+.EXE");
    checker.ShutDownFuzzyMatchApp();

    QThread* t =new SetupThread(this, localFilePath_);
    CHECK_CONNECT_ERROR(connect(t, &QThread::started, []() {
        qDebug() << "succeed to create process ";
        }));
    CHECK_CONNECT_ERROR(connect(t, &QThread::finished, [=]() {
        bool ret = CheckVersionFileAfterSetup();
        if (!ret)
        {
            qDebug() << "VersionFile error";
        }
        else {
            //update Ui here
        }
        }));
    CHECK_CONNECT_ERROR(connect(t, &QThread::finished, t, &QObject::deleteLater));
    t->start();
    return true;
}

QString DownloadInfoWidget::MakeDownloadHeadway()
{
    return MakeDownloadHeadway(bytesDown_, totalSize_);
}

QString DownloadInfoWidget::MakeDownloadHeadway(int64_t readed, int64_t total)
{
    if (total < 10456)
    {
        QString result = QString::number(ToKByte(readed), 'f', 2);
        result += "KB/";
        result += QString::number(ToKByte(total), 'f', 2) + "KB";
        return result;
    }
    else{
        QString result = QString::number(ToMByte(readed), 'f', 2);
        result += "MB/";
        result += QString::number(ToMByte(total), 'f', 2) + "MB";
        return result;
    }
    
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
    if (bytesTotal == 0)
    {
        qWarning() << u8"Ƶ������������Qt��bug";
        return;
    }
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
    long long speed = increment/secondElepsed;//����̫��
    QString speedString = MakeDownloadSpeed(speed);
    emit notify_stateLabel(speedString);

    //fix me, ����ʣ��ʱ��
    int secondLeftEstimated = (bytesTotal - bytesReceived) / (increment / secondElepsed);
    emit notify_timeLabel(MakeDurationToString(secondLeftEstimated));

    bytesDown_ = bytesReceived + size_lastDownloadTask;
    if (bytesDown_ == totalSize_)
    {
        qDebug() << bytesDown_;
    }
    qDebug() << bytesDown_;
    emit notify_sizeInfo(MakeDownloadHeadway());
}

//fix me, �ж�״̬����
void DownloadInfoWidget::UpdatePlayButton(bool stopped)
{
    if (stopped)
    {
        downloadButton_->show();
        pauseButton_->hide();
        //debug
        if (downloadState_ == DownloadState::Downloading)
        {
            qDebug() << u8"״̬��ӦΪDownloading";
        }
    }
    else
    {
        downloadButton_->hide();
        pauseButton_->show();
        //debug
        if (downloadState_ != DownloadState::Downloading)
        {
            qDebug() << u8"״̬ӦΪDownloading";
        }
    }
}

//�ض�һ���ļ�
void DownloadInfoWidget::LoadingProgressForBreakPoint()
{
    QFileInfo fileInfo(localFilePath_);
    if (!fileInfo.exists())
    {
        bytesDown_ = 0;
        progressBar_->setValue(0);
        leftTimeEstimated_->setText("--");
        fileDownloadHeadway_->setText(MakeDownloadHeadway());
        return;
    }
    else
    {
        bytesDown_=fileInfo.size();
        qDebug() << bytesDown_;
        if (bytesDown_ > totalSize_)
        {
            qWarning() << "bad file size";
            downloadStatusLabel_->setText(u8"�ļ���С���󣬽�����������");
            QFile f(localFilePath_);
            f.open(QIODevice::WriteOnly);
            f.remove();
            f.close();
        }
        else if (bytesDown_ == totalSize_)
        {
            downloadStatusLabel_->setText(u8"�����");
            downloadState_ = DownloadState::Finished;
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

void DownloadInfoWidget::UpdateUiAccordingToState()
{
    //������д
}

bool DownloadInfoWidget::CheckVersionFileAfterSetup()
{
    //�����������ײ�
    return true;
}

void DownloadInfoWidget::AddMenuItems()
{
    QMenu* lableMenu = new QMenu(this);
    //lableMenu->addAction(QIcon(":/images/play.png"), u8"��ʼ");
    //lableMenu->addSeparator();
    //lableMenu->addAction(QIcon(":/images/pause.png"), u8"��ͣ");
    //lableMenu->addSeparator();
    lableMenu->addAction(QIcon(":/images/close-gray.png"), u8"ɾ��");
    lableMenu->addSeparator();
    lableMenu->addAction(QIcon(":/images/folder.png"), u8"�������ļ���");
    CHECK_CONNECT_ERROR(connect(this, &QWidget::customContextMenuRequested,
        [=](const QPoint& /*pos*/) {
            lableMenu->exec(QCursor::pos());
        }));
    (connect(lableMenu, &QMenu::triggered, [=](QAction* action) {
        QString str = action->text();
        if (str == u8"��ʼ")
            StartDownloadTask();
        else if (str == u8"��ͣ")
            PauseDownloadTask();
        else if (str == u8"ɾ��")
            CancelDownloadTask();
        else if (str == u8"�������ļ���")
        {
            if(QFile::exists(localFilePath_))
                OpenLocalPath(localFilePath_);
            else {
                auto index=localFilePath_.lastIndexOf("/");
                auto folderPath = localFilePath_.mid(0, index);
                OpenLocalPath(folderPath);
            }
        }
        }));
}