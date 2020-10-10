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

//Widget as a item in QListWidget
class ComparisonDownloadInfoWidget : public QWidget
{
    Q_OBJECT

public:
    ComparisonDownloadInfoWidget(QWidget* parent, const QString& fileName, qint64 fileSize, const QString& url);
    ~ComparisonDownloadInfoWidget() = default;

    enum class DownloadState
    {
        NotStarted,
        Downloading,
        Paused,
        Cancel,
        Error,
        Finished
    };

public:
    //bool StartDownloadTask();
    //bool PauseDownloadTask();
    //bool CancelDownloadTask();

private:
    QString url_;
    QString fileName_;
    QString localFilePath_;

    QLabel* fileNameLabel_;//���е���ң������ļ���Ӧ���ǰ�������Ӧ�úͰ汾һ��?
    QLabel* leftTimeEstimated_;
    QLabel* downloadStatusLabel_;
    QLabel* fileDownloadHeadway_;

    QProgressBar* progressBar_;
    QProgressBar* setupProgressBar_;

    QPushButton* pauseButton_;
    QPushButton* downloadButton_;

    qint64 totalSize_;//ʼ�մ����ļ�������С�����Ǳ��������С
    qint64 bytesDown_;

    QNetworkReply* reply_;
    std::unique_ptr<QFile> file_;
    DownloadState downloadState_;
    bool isBreakPointTranSupported_;
    QNetworkAccessManager QNAManager_;
};



