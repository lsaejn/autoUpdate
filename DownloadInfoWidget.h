#pragma once
#include <QWidget>
#include <QThread>
#include <atomic>
#include <QFile>
#include <QFileInfo>
#include "QtNetwork/qnetworkaccessmanager.h"
#include "QtNetwork/qnetworkreply.h"
#include "QtNetwork/qnetworkrequest.h"

/*
�ҿ������ˣ�gui������������߼��ָ�...
*/
class QLabel;
class QProgressBar;
class QPushButton;
class QFile;
class QFileInfo;

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
    ~DownloadInfoWidget();

    enum WebFileType
    {
        Exe = 0,//����
        Iso =1,//����
        Other
    };

    enum DownloadState
    {
        Normal,
        Stopped,
        Paused
    };

private:
    bool StartDownloadTask();
    bool CancelDownloadTask();
    bool OpenDownloadFolder();
    void StartRequest(const QUrl& url);
    void httpFinished();
    void httpReadyRead();
    bool isTimeToUpdate(double& second);
    QString MakeDownloadHeadway();
    QString MakeDownloadHeadway(int64_t reader, int64_t total);
    QString MakeDurationToString(int seconds);
    void UpdateChildWidgets(qint64 bytesReceived, qint64 bytesTotal);
    void UpdatePlayButton(bool stopped=true);
    bool IsFileExist();
    uint64_t GetLocalFileSize(const QString &s);//����û�а취����ļ���Ч��

private:
    QLabel* fileDownloadHeadway_;//��λ����ʾ�����ؽ���
    QLabel* leftTimeEstimated_;
    QLabel* state_;
    QProgressBar* bar_;
    QPushButton* downloadSwitch_;
    QPushButton* pauseSwitch_;
    QString url_;
    QLabel* fileName_;//���е���ң������ļ���Ӧ���ǰ�������Ӧ�úͰ汾һ��?
    uint64_t sizeDown_;
    uint64_t totalSize_;
    std::unique_ptr<QFile> file_;
    QNetworkAccessManager qnam;
    QNetworkReply* reply;
    DownloadState downloadState_;
    bool stopped_;
};


