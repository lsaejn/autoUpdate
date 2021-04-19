#include <QDir>
#include <QLabel>
#include <QMenu>
#include <QLayout>
#include <QPushbutton>
#include <QProgressBar>
#include <QMessageBox>
#include <QProcess>
#include <QEventLoop>
#include <QThread>


#include <thread>

#include "DownloadInfoWidget.h"
#include "PackageListWidget.h"
#include "TaskThread.h"
#include "SetupHelper.h"
#include "Alime/ScopeGuard.h"
#include "Alime/time/Timestamp.h"
#include "Alime/time/Duration.h"
#include "AppUtility.h"
#include "CustomWidget.h"


std::atomic_bool  DownloadInfoWidget::Setuping_ = false;

DownloadInfoWidget::DownloadInfoWidget(QWidget* _parent, const QString& _fileName, 
    qint64 _fileSize, const QUrl& _url, const QUrl& instructionUrl, PackType ty)
    :QWidget(_parent),
    url_(_url),
    instructionUrl_(instructionUrl),
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
    isBreakPointTranSupported_(true),
    packType_(ty)
{
    localFilePath_ = GetDownloadFolder()+fileName_;
    setContextMenuPolicy(Qt::CustomContextMenu);

    AddMenuItems();

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->addSpacing(5);
    mainLayout->setSpacing(0);

    //for debug
    setAttribute(Qt::WA_StyledBackground, true);
    //setStyleSheet("border:2px solid #014F84; background-color:rgb(255,0,0)");

    {
        QLabel* typeLabel = new QLabel(this);
        if (packType_==PackType::UpdatePack
            || packType_ ==PackType::Image)
        {
            typeLabel->setObjectName("PackLabel");
            typeLabel->setText(u8"������");
        }
        else
        {
            typeLabel->setObjectName("PackLabel");
            typeLabel->setText(u8"������");
        }
        mainLayout->addWidget(typeLabel);
        mainLayout->addSpacing(10);
    }

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
        //progressBar_->setFixedHeight(20);
        progressBar_->setRange(0, 100);

        setupProgressBar_ = new QProgressBar(this);
        setupProgressBar_->setFixedWidth(240);
        setupProgressBar_->setRange(0, 0);
        setupProgressBar_->setVisible(false);
        downloadStateBox->addWidget(setupProgressBar_);

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

    {
        downloadButton_ = new QPushButton(this);
        downloadButton_->setObjectName("ItemPlay");
        downloadButton_->setText(u8"һ������");
        connect(downloadButton_, &QPushButton::clicked, [this, _parent] {
            //�����Զ���ģ̬�Ի���
            //CustomWidget wid(this);
            //wid.exec();
            if (IsDownLoading() || IsSetuping())
                return;
            //fix me, ��һ���հ�����
            auto pptr = dynamic_cast<PackageListWidget*>(_parent);
            if (pptr->HasSetupItem())
            {
                downloadStatusLabel_->setText(u8"��һ����װ����ִ��");
                ShowWarningBox(u8"��������", u8"��һ����װ����ִ��", u8"ȷ��");
                return;
            }
            StartDownloadTask();
            });

        //��ͣ��ť, �°汾��ȥ��
        pauseButton_= new QPushButton(this);
        pauseButton_->setObjectName("ItemPlayPause");
        pauseButton_->setToolTip(u8"��ͣ");
        pauseButton_->setText(u8"һ������");
        pauseButton_->hide();
        CHECK_CONNECT_ERROR(connect(pauseButton_, &QPushButton::clicked, [this]()
            {
                PauseDownloadTask();
            }));

        //fix,me ɾ����ť, �°汾��ȥ��
        QPushButton* deleteLocalFile = new QPushButton(this);
        deleteLocalFile->setObjectName("ItemDelete");
        deleteLocalFile->setToolTip(u8"ɾ���ļ�");
        deleteLocalFile->setVisible(false);
        CHECK_CONNECT_ERROR(connect(deleteLocalFile, &QPushButton::clicked, [this]() {
            CancelDownloadTask();
            }));

        //���°�ť���°汾��ȥ��
        QPushButton* setupBtn = new QPushButton(this);
        setupBtn->setObjectName("ItemSetup");
        setupBtn->setToolTip(u8"��ʼ��װ");
        setupBtn->setVisible(false);
        CHECK_CONNECT_ERROR(connect(setupBtn, &QPushButton::clicked, [this]()
            {
                this->DoSetup();
            }));

        //����˵����ť
        QPushButton* instructionBtn = new QPushButton(this);
        instructionBtn->setObjectName("instructionButton");
        instructionBtn->setText(u8"����˵��");
        connect(instructionBtn, &QPushButton::clicked, [this](){
                auto url=instructionUrl_.toString().toStdWString();
                if(!url.empty())
                    ShellExecute(NULL, L"open", url.c_str(), L"", L"", SW_SHOW);
            });

        mainLayout->addWidget(downloadButton_);
        mainLayout->addSpacing(15);
        mainLayout->addWidget(instructionBtn);
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
    if(downloadState_!= DownloadState::NotStarted
        && downloadState_ != DownloadState::Finished )
    {
        qCritical() << u8"״̬����,�������߼�";
        return false;
    }
    QUrl newUrl = url_;//for local debug
    
    if (newUrl.isEmpty() || !newUrl.isValid() || newUrl.fileName().isEmpty())
    {
        qCritical() << QString("Invalid URL: %1: %2").arg(newUrl.toString(), newUrl.errorString());
        downloadStatusLabel_->setText(u8"�޷�����");
        return false;//ά����Ա�ϴ����ļ�����,url��Ч
    }

    if (QFile::exists(localFilePath_))
    {
        QFileInfo fileInfo(localFilePath_);
        auto fileSize=fileInfo.size();

        if (fileSize > totalSize_)
        {
            qDebug() << u8"Invalid local file size, should not larger than targetSize";
            file_.reset();
            QFile::remove(localFilePath_);
            fileInfo = QFileInfo(localFilePath_);
            fileSize = fileInfo.size();
        }
        else if (fileSize == totalSize_)//����û�а취��֤�������ļ�����Ч��,�ֽ������ˣ���Ϊ�ɹ�
        {
            DoSetup();
            return true;
        }
    }

    LoadingProgressForBreakPoint();

    file_ = openFileForWrite(localFilePath_);
    if (!file_)
    {
        ShowWarningBox(u8"�޷����ļ�", u8"�ļ����ܱ�ռ�ã��޷�д��", u8"ȷ��");
        return false;
    }
           
    downloadState_ = DownloadState::Downloading;
    downloadStatusLabel_->setText(u8"��������...");
    StartRequest(newUrl);
    return true;
}


void DownloadInfoWidget::StartRequest(const QUrl& requestedUrl)
{
    QUrl url = requestedUrl;

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, false);
    // ���֧�ֶϵ�����������������ͷ��Ϣ
    if (isBreakPointTranSupported_)
    {
        QString strRange = QString("bytes=%1-").arg(bytesDown_);
        qDebug() << "StartRequest, url:"<< url.toString()<<" ,bytesDown_="<<bytesDown_;
        request.setRawHeader("Range", strRange.toLatin1());
    }
    reply_ = QNAManager_.get(QNetworkRequest(request));
    bool succ=connect(reply_, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), 
        this, &DownloadInfoWidget::httpError);
    CHECK_CONNECT_ERROR(connect(reply_, &QNetworkReply::finished, this, &DownloadInfoWidget::httpFinished));
    CHECK_CONNECT_ERROR(connect(reply_, &QIODevice::readyRead, this, &DownloadInfoWidget::httpReadyRead));
    CHECK_CONNECT_ERROR(connect(reply_, &QNetworkReply::downloadProgress, this, &DownloadInfoWidget::UpdateChildWidgets));

    //�������ź�����Ϊ���ؽ����ᶪ���������̣߳���ʱֱ��һճ...
    CHECK_CONNECT_ERROR(connect(this, &DownloadInfoWidget::notify_progressInfo, progressBar_, &QProgressBar::setValue));
    CHECK_CONNECT_ERROR(connect(this, &DownloadInfoWidget::notify_sizeInfo, fileDownloadHeadway_, &QLabel::setText));
    CHECK_CONNECT_ERROR(connect(this, &DownloadInfoWidget::notify_stateLabel, downloadStatusLabel_, &QLabel::setText));
    CHECK_CONNECT_ERROR(connect(this, &DownloadInfoWidget::notify_timeLabel, leftTimeEstimated_, &QLabel::setText));
}

void DownloadInfoWidget::httpError(QNetworkReply::NetworkError errorCode)
{
    qDebug() << "error happened, NetworkError is"<<errorCode;
}

//�������/��ͣ/ȡ��/����
void DownloadInfoWidget::httpFinished()
{
    bool redirected = false;
    ALIME_SCOPE_EXIT{
        if (!redirected)
        {
            reply_->deleteLater();
            reply_ = nullptr;
        }
    };
    {
        file_->flush();
        file_->close();
        file_.reset();
    }

    //��ʱ��֧���ض���
    /*
    {
        QVariant redirectionTarget = reply_->attribute(QNetworkRequest::RedirectionTargetAttribute);
        //http://203.187.160.133:9011/update.pkpm.cn/PKPM2010/Info/pkpmSoft/UpdatePacks/V5.2.1Setup.exe
        //redirectionTarget = "http://203.187.160.133:9011/update.pkpm.cn/c3pr90ntc0td/PKPM2010/Info/pkpmSoft/UpdatePacks/V5.2.1Setup.exe";
        if (!redirectionTarget.isNull())
        {
            redirected = true;
            const QUrl redirectedUrl = redirectionTarget.toUrl();
            int statusCode = reply_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "redirected " << statusCode;
            qDebug() << "status code is " << statusCode;
            file_ = openFileForWrite(localFilePath_);
            if (!file_)
            {
                qWarning() << "could not open file";
                reply_->deleteLater();
                reply_ = NULL;
                return;
            }
            reply_->deleteLater();
            reply_ = NULL;
            //fix me, ֧���ض���
            StartRequest(redirectedUrl);
            return;
        }
    }
    */

    if (bytesDown_ == totalSize_)
    {
        downloadState_ = DownloadState::Finished;
        downloadStatusLabel_->setText(u8"�����");
        DoSetup();
    }
    else if (bytesDown_ < totalSize_ )//we retry connecting here
    {
        if (reply_->error())
        { 
            qDebug() << reply_->errorString();
            if (DownloadState::Paused== downloadState_)
            {
                downloadStatusLabel_->setText(u8"��ͣ��...");
                downloadState_ = DownloadState::NotStarted;
                qDebug()<<u8"�û��ж�������  "<< QString::number(bytesDown_);
            }
            else if (DownloadState::Cancel == downloadState_)
            {
                qDebug(u8"�û�ȡ����ɾ�����ļ�����");
                downloadStatusLabel_->setText(u8"����ȡ��");
                QFile::remove(localFilePath_);
                LoadingProgressForBreakPoint();
                downloadState_ = DownloadState::NotStarted;
            }
            else if(DownloadState::Downloading == downloadState_)
            {
                downloadStatusLabel_->setText(u8"���س���");
                qDebug(u8"�Զ˹ر�����/�����ж�");
                downloadState_ = DownloadState::NotStarted;
            }
            else
            {
                qDebug(u8"״̬�߼�����");
            }
            UpdatePlayButton();
        }
    }
    else
    {
        qDebug(u8"�ļ���С����, �û��ֶ��޸����ļ����ƻ������ص����ݴ���");
        downloadState_ = DownloadState::NotStarted;
        downloadStatusLabel_->setText(u8"��������");
        UpdatePlayButton();
    }
    leftTimeEstimated_->setText("--");
}

void DownloadInfoWidget::httpReadyRead()
{
    if (downloadState_ != DownloadState::Downloading)
    {
        qWarning() << u8"state error";
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
    //fix me, ����ͳһ��readCallback����£����Կ�����ô��
    //��Ҳ���¶�������ʱ�򣬸���������������Ǵ���ġ�
    //��������������ͬʱֻ�ܸ���һ���ļ�
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
    if (downloadState_ == DownloadState::NotStarted 
        || downloadState_ == DownloadState::Finished)
    {
        QFile::remove(localFilePath_);
        LoadingProgressForBreakPoint();
        downloadState_ = DownloadState::NotStarted;
        downloadStatusLabel_->setText(u8"��ͣ��...");
        UpdatePlayButton(true);
        return true;
    }
    else if (downloadState_ == DownloadState::Downloading)
    {
        //we update State in httpFinish as file was writing
        downloadState_ = DownloadState::Cancel;
        reply_->abort();
        return true;
    }
    else
    {
        qWarning() << "bad logic error in CancelDownloadTask";
        //UpdatePlayButton(true); or not
        return false;
    }
    return true;
}

bool DownloadInfoWidget::PauseDownloadTask()
{
    if (downloadState_ == DownloadState::Downloading)
    {
        downloadState_ = DownloadState::Paused;
        reply_->abort();
        //UpdatePlayButton(true);
        return true;
    }
    else
    {
        qWarning() << "bad logic error in PauseDownloadTask";
        return false;
    }
}

#include "UnZipper.h"
bool DownloadInfoWidget::DoSetup()
{
    if (DownloadState::Finished != downloadState_)
    {
        ShowWarningBox(u8"��������", u8"���������ļ�", u8"�˳�");
        //std::abort(); or not
        return false;
    }
    if (SetupThread::HasInstance())
    {
        downloadStatusLabel_->setText(u8"��һ����װ����ִ��");
        //ShowWarningBox(u8"��������", u8"����ִ����һ����װ", u8"�˳�");
        //std::abort(); or not
        return false;
    }

    //���ǹر��������������
    ProcessManager checker;
    checker.SetMatchReg(L"PKPMMAIN.EXE");
    bool ret = checker.AssurePkpmmainClosed();
    if (!ret)
        return false;
    checker.SetMatchReg(L"PKPM[\\d]{4}V[\\d]+.EXE");
    ret=checker.ShutDownFuzzyMatchApp();
    if (!ret)
        return false;

    SetupThread* setuper = nullptr;
    if (localFilePath_.endsWith(".exe", Qt::CaseInsensitive))
    {
        setuper = new ExeSetupThread(this, localFilePath_);
    }
    else if (localFilePath_.endsWith(".zip", Qt::CaseInsensitive))
    {
        setuper = new ZipSetupThread(this, localFilePath_);
    }
    else
    {
        OpenLocalPath(localFilePath_);
        SetupFinished();
        return false;
    }

    assert(setuper);
    CHECK_CONNECT_ERROR(connect(setuper, &QThread::started, this, &DownloadInfoWidget::SetupStarted));
    CHECK_CONNECT_ERROR(connect(setuper, &QThread::finished, this, &DownloadInfoWidget::SetupFinished));
    CHECK_CONNECT_ERROR(connect(setuper, &QThread::finished, setuper, &QObject::deleteLater));
    connect(setuper, &SetupThread::TaskFinished, this, &DownloadInfoWidget::ShowTipsWhenSetupFinished, Qt::QueuedConnection);
    setuper->start();
    return true;
}

//��Ҫ�ĳ�static
void DownloadInfoWidget::ShowTipsWhenSetupFinished(int errorCode)
{
    if (!errorCode)
    {
        qWarning() << "in function ShowTipsWhenSetupFinished, error Code is"<< errorCode;
        return;
    }
    else {
        ShowWarningBox(u8"����", u8"��װʧ��", u8"ȷ��");
    }
}

void DownloadInfoWidget::ShowSetupProgress(bool visible)
{
    if (!this)
    {
        qWarning() << "Fatal error, you know";
        return;
    }
        
    if (visible)
    {
        progressBar_->setVisible(false);
        setupProgressBar_->setVisible(true);
        downloadStatusLabel_->setText(u8"���ڰ�װ...");
    }
    else
    {
        progressBar_->setVisible(true);
        setupProgressBar_->setVisible(false);
        downloadStatusLabel_->setText(u8"�����");
    }
}

void DownloadInfoWidget::SetupStarted()
{
    Setuping_ = true;
    ShowSetupProgress(true);
}

#include <QListWidget>
void  DownloadInfoWidget::SetupFinished()
{
    Setuping_ = false;
    ShowSetupProgress(false);
    emit finishSetup();
    
}

bool DownloadInfoWidget::IsSetuping()
{
    return Setuping_;
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
        qWarning() << u8"bytesTotal=0. connection timeout?";
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
        qDebug() << u8"�������"<<bytesDown_;
    }
    //qDebug() << u8"������:"<<bytesDown_;
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
        if (bytesDown_ > totalSize_)
        {
            qWarning() << "LoadingProgressForBreakPoint() bad file size";
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
    lableMenu->addAction(QIcon(":/images/close-gray.png"), u8"ɾ���������ļ�");
    lableMenu->addSeparator();
    lableMenu->addAction(QIcon(":/images/folder.png"), u8"�������ļ���");
    CHECK_CONNECT_ERROR(connect(this, &QWidget::customContextMenuRequested,
        [=](const QPoint& /*pos*/) {
            lableMenu->exec(QCursor::pos());
        }));
    CHECK_CONNECT_ERROR(connect(lableMenu, &QMenu::triggered, [=](QAction* action) {
        QString str = action->text();
        if (str == u8"ɾ���������ļ�")
        {
            //ɾ���������ǲ���Ϊ���湦��
            CancelDownloadTask();
        }
            
        else if (str == u8"�������ļ���")
        {
            if(QFile::exists(localFilePath_))
                OpenLocalPath(localFilePath_);
            else
            {
                auto index=localFilePath_.lastIndexOf("/");
                auto folderPath = localFilePath_.mid(0, index);
                OpenLocalPath(folderPath);
            }
        }
        }));
}

PackType DownloadInfoWidget::GetPackType()
{
    return packType_;
}

bool DownloadInfoWidget::IsFinished()
{
    return  bytesDown_ ==totalSize_ /*&& DownloadState::Finished*/;
}

bool DownloadInfoWidget::IsDownLoading()
{
    return downloadState_ == DownloadState::Downloading;
}

void DownloadInfoWidget::SetPackFlag(PackType ty)
{
    packType_ = ty;
}