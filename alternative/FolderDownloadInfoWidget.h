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

#include "FolderCompare.h"

class QLabel;
class QProgressBar;
class QPushButton;
class QFile;
class QFileInfo;

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
        replacing,
        Finished
    };

public:
    bool StartDownloadTask();
    bool PauseDownloadTask();
    bool CancelDownloadTask();

private:
    QString webRootUrl_;
    QString rootPath_;

    QLabel* versionNameLabel_;//���е���ң������ļ���Ӧ���ǰ�������Ӧ�úͰ汾һ��?
    QLabel* downloadStatusLabel_;

    QProgressBar* progressBar_;

    QPushButton* pauseButton_;
    QPushButton* downloadButton_;

    uint fileCountFinished;
    QVector<QString> filesToDownload_;

    QNetworkReply* reply_;
    std::unique_ptr<QFile> file_;
    DownloadState downloadState_;
    bool isBreakPointTranSupported_;
    QNetworkAccessManager QNAManager_;
    ResourceCompare Comparer_;
};



