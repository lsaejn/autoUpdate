#pragma once
#include <QWidget>
#include <QThread>
#include <QFile>
#include <QFileInfo>

#include "QtNetwork/qnetworkaccessmanager.h"
#include "QtNetwork/qnetworkreply.h"
#include "QtNetwork/qnetworkrequest.h"

#include <atomic>

class QLabel;
class QProgressBar;
class QPushButton;
class QFile;
class QFileInfo;

/*
需要StateManager+FileManager
*/


class DownloadInfoWidget : public QWidget
{
    Q_OBJECT
signals:
    void notify_progressInfo(double);
    void notify_sizeInfo(QString);
    void notify_playButton(bool);
    void notify_stateLabel(QString);
    void notify_timeLabel(QString);
public:
    DownloadInfoWidget(QWidget* parent, const QString& fileName, uint64_t fileSize, const QString& url);
    ~DownloadInfoWidget()=default;

    enum WebFileType
    {
        Exe = 0,//补丁
        Iso =1,//光盘
        Other
    };

    enum DownloadState
    {
        NotStarted,
        Interrupted,
        Canceled,
        Paused,
        Finished
    };

private:
    bool StartDownloadTask();
    bool CancelDownloadTask();
    bool PauseDownloadTask();
    bool OpenDownloadFolder();
    
    void httpFinished();
    void httpReadyRead();
    void StartRequest(const QUrl& url);
    bool isTimeToUpdate(double& second);

    QString MakeDownloadHeadway();
    QString MakeDownloadHeadway(int64_t reader, int64_t total);
    QString MakeDurationToString(int seconds);

    void LoadingProgressForBreakPoint();
    void UpdateChildWidgets(qint64 bytesReceived, qint64 bytesTotal);
    void UpdatePlayButton(bool stopped=true);
    bool IsFileExist();
    uint64_t GetLocalFileSize(const QString &s);//我们没有办法检查文件有效性
    std::unique_ptr<QFile> openFileForWrite(const QString& fileName);

private:
    QString url_;
    QString fileName_;
    QString localFilePath_;

    QLabel* downloadStatusLabel_;
    QLabel* leftTimeEstimated_;
    QLabel* fileDownloadHeadway_;
    QProgressBar* progressBar_;
    QPushButton* pauseButton_;
    QPushButton* downloadButton_;
    QLabel* fileNameLabel_;//我有点混乱，更新文件名应该是包名还是应该和版本一样?

    uint64_t bytesDown_;
    uint64_t totalSize_;//始终代表整个文件大小，而非本次需下载的总大小

    std::unique_ptr<QFile> file_;
    QNetworkAccessManager QNAManager_;
    QNetworkReply* reply_;
    DownloadState downloadState_;
    bool isBreakPointTranSupported_=true;


    struct stateManager
    {
        void Notify();
    };
};


