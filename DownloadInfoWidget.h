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
#include <functional>

class QLabel;
class QProgressBar;
class QPushButton;
class QFile;
class QFileInfo;

using CheckCallBack=std::function<bool()> ;
/*
需要StateManager+FileManager
来管理控件
downloadState_ 
downloadStatusLabel_
*/

//Widget as a item in QListWidget
class DownloadInfoWidget : public QWidget
{
    Q_OBJECT
signals:
    //没有必要的信号
    void notify_progressInfo(double);
    void notify_sizeInfo(QString);
    void notify_playButton(bool);
    void notify_stateLabel(QString);
    void notify_timeLabel(QString);

    void finishDownload();
    void finishSetup();
    void errorDownload();
public slots:
    void ShowTipsWhenSetupFinished(int);
    void SetupStarted();
    void SetupFinished();
    
public:
    DownloadInfoWidget(QWidget* parent, const QString& fileName, qint64 fileSize, const QUrl& url);
    ~DownloadInfoWidget()=default;

    enum class WebFileType
    {
        Exe = 0,//补丁
        Iso =1,//光盘
        Other
    };

    //调试用,时间比较吃紧，我们改成单线程跑状态机
    //NotStarted，Finished是常态;Downloading是二者的中间状态；另外四个是中间状态转常态的顺时状态，方便更新控件
    //
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

//用户操作设为public，方便记忆
public:
    bool StartDownloadTask();
    bool PauseDownloadTask();
    bool CancelDownloadTask();
    bool DoSetup();

public:
    bool IsUpdatePackage();
    bool IsFinished();
    bool IsDownLoading();
    void SetCheckCallBack(CheckCallBack f);
private:
    void httpFinished();
    void httpReadyRead();
    void AddMenuItems();
    void SetPackFlag(int type);
    bool IsAutoSetupRunning();
    void ShowSetupProgress(bool);
    void UpdateUiAccordingToState();
    bool CheckVersionFileAfterSetup();
    void StartRequest(const QUrl& url);
    bool isTimeToUpdate(double& second);
    void httpError(QNetworkReply::NetworkError);

    QString MakeDownloadHeadway();
    QString MakeDownloadHeadway(int64_t reader, int64_t total);
    QString MakeDurationToString(int seconds);

    void LoadingProgressForBreakPoint();
    void UpdateChildWidgets(qint64 bytesReceived, qint64 bytesTotal);
    void UpdatePlayButton(bool stopped=true);
    std::unique_ptr<QFile> openFileForWrite(const QString& fileName);
    

private:
    QUrl url_;
    QString fileName_;
    QString localFilePath_;

    QLabel* fileNameLabel_;//我有点混乱，更新文件名应该是包名还是应该和版本一样?
    QLabel* leftTimeEstimated_;
    QLabel* downloadStatusLabel_;
    QLabel* fileDownloadHeadway_;

    QProgressBar* progressBar_;
    QProgressBar* setupProgressBar_;

    QPushButton* pauseButton_;
    QPushButton* downloadButton_;
    
    qint64 totalSize_;//始终代表文件完整大小，而非本次任务大小
    qint64 bytesDown_;
    
    QNetworkReply* reply_;
    std::unique_ptr<QFile> file_;
    //fix me, use function instead
    DownloadState downloadState_;
    bool isBreakPointTranSupported_;
    QNetworkAccessManager QNAManager_;
    int redirectTimes_;
    int retryTimes_;

    int packFlag_;

    CheckCallBack autoRuningFunc_;
};


