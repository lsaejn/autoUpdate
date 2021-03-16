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
#include <string>

#include "Alime/Console.h"

class QLabel;
class QProgressBar;
class QPushButton;
class QFile;
class QFileInfo;

using CheckCallBack=std::function<bool()> ;
/*
��ҪStateManager+FileManager
������ؼ�
downloadState_ 
downloadStatusLabel_
*/

//Widget as a ListItem in QListWidget
class DownloadInfoWidget : public QWidget
{
    Q_OBJECT
signals:
    //û�б�Ҫ���ź�
    void notify_progressInfo(double);
    void notify_sizeInfo(QString);
    void notify_playButton(bool);
    void notify_stateLabel(QString);
    void notify_timeLabel(QString);

    void finishDownload();
    void finishSetup(bool);//ʹ��ѹ������Ľ���������ǲ�֪���ǲ��ǰ�װ�ɹ�
    void errorDownload();

public slots:
    void ShowTipsWhenSetupFinished(int);
    void SetupStarted();
    void SetupFinished();
    
public:
    DownloadInfoWidget(QWidget* parent, const QString& fileName,
        qint64 fileSize, const QUrl& url, const QUrl& instructionUrl, bool isUpdatePack=true);
    
    ~DownloadInfoWidget()
    {
        std::wstring show;
        show = std::to_wstring((int)this);
        Alime::Console::WriteLine(L"DownloadInfo's address is " + show);
        disconnect();
    }

    enum class WebFileType
    {
        Exe = 0,//exe
        Iso =1,//����
        Zip=2,//zip
        Other
    };

    //������,ʱ��ȽϳԽ������Ǹĳɵ��߳���״̬��
    //NotStarted��Finished�ǳ�̬;Downloading�Ƕ��ߵ��м�״̬�������ĸ����м�״̬ת��̬��˳ʱ״̬��������¿ؼ�
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

//�û�������Ϊpublic���������
public:
    bool StartDownloadTask();
    bool PauseDownloadTask();
    bool CancelDownloadTask();
    bool DoSetup();
    bool IsUpdatePackage();
    bool IsFinished();//download
    bool IsSetuping();
    bool IsDownLoading();
    void SetCheckCallBack(CheckCallBack f);
    void SetPackFlag(bool isUpdatePackage);
private:
    void httpFinished();
    void httpReadyRead();
    void AddMenuItems();
    
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
    QUrl instructionUrl_;
    QString fileName_;
    QString localFilePath_;

    QLabel* fileNameLabel_;//�ļ���
    QLabel* leftTimeEstimated_;
    QLabel* downloadStatusLabel_;//��ʾ����/��װ״̬
    QLabel* fileDownloadHeadway_;

    QProgressBar* progressBar_;
    QProgressBar* setupProgressBar_;

    QPushButton* pauseButton_;
    QPushButton* downloadButton_;
    
    qint64 totalSize_;//ʼ�մ����ļ�������С�����Ǳ��������С
    qint64 bytesDown_;
    
    QNetworkReply* reply_;
    std::unique_ptr<QFile> file_;
    //fix me, use function instead
    DownloadState downloadState_;
    QNetworkAccessManager QNAManager_;

    bool Setuping_;
    bool isUpdatePack_;
    bool isBreakPointTranSupported_;
    
    int retryTimes_;
};


