#pragma once
#include <QWidget>
#include <QThread>
#include <QFile>
#include <QFileInfo>

#include "QtNetwork/qnetworkaccessmanager.h"
#include "QtNetwork/qnetworkreply.h"
#include "QtNetwork/qnetworkrequest.h"

#include <atomic>
#include <memory>

class QLabel;
class QProgressBar;
class QPushButton;
class QFile;
class QFileInfo;

/*
��ҪStateManager+FileManager
������ؼ�
downloadState_ 
downloadStatusLabel_
*/

//Widget as a item in QListWidget
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

    enum class WebFileType
    {
        Exe = 0,//����
        Iso =1,//����
        Other
    };

    //������,ʱ��ȽϳԽ������Ǹĳɵ��߳���״̬��
    enum class DownloadState
    {
        NotStarted,
        Downloading,
        Paused,
        Cancel,
        Interrupted,
        Error,
        Finished
    };

private:
    bool StartDownloadTask();
    bool CancelDownloadTask();
    bool PauseDownloadTask();
    bool DoSetup();
    
    void httpFinished();
    void httpReadyRead();
    void StartRequest(const QUrl& url);
    bool isTimeToUpdate(double& second);
    void UpdateUiAccordingToState();
    bool CheckVersionFileAfterSetup();

    QString MakeDownloadHeadway();
    QString MakeDownloadHeadway(int64_t reader, int64_t total);
    QString MakeDurationToString(int seconds);

    void LoadingProgressForBreakPoint();
    void UpdateChildWidgets(qint64 bytesReceived, qint64 bytesTotal);
    void UpdatePlayButton(bool stopped=true);
    std::unique_ptr<QFile> openFileForWrite(const QString& fileName);

private:
    QString url_;
    QString fileName_;
    QString localFilePath_;

    QLabel* fileNameLabel_;//���е���ң������ļ���Ӧ���ǰ�������Ӧ�úͰ汾һ��?
    QLabel* leftTimeEstimated_;
    QLabel* downloadStatusLabel_;
    QLabel* fileDownloadHeadway_;

    QProgressBar* progressBar_;
    QPushButton* pauseButton_;
    QPushButton* downloadButton_;
    
    uint64_t bytesDown_;
    uint64_t totalSize_;//ʼ�մ��������ļ���С�����Ǳ��������ص��ܴ�С
 
    QNetworkReply* reply_;
    std::unique_ptr<QFile> file_;
    DownloadState downloadState_;
    bool isBreakPointTranSupported_;
    QNetworkAccessManager QNAManager_;

};


