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

/*
�ļ��в������ʹ�ò�ͬ����ʽ
�汾����+[�������ص��ļ���+�ٶ�]+������+��ʼ+��ͣ+����
*/

class ComparisonDownloadInfoWidget : public QWidget
{
    Q_OBJECT

public:
    ComparisonDownloadInfoWidget(QWidget* parent, const QString& version);
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
    QString rootPath_;
    QString webRootUrl_;
    QString* fileDownloading_;//�������·�����ļ���
    
    QLabel* versionNameLabel_;
    QLabel* speed_;

    QProgressBar* progressBar_;
    QLabel* fileDownloadingLabel_;

    QPushButton* startButton_;
    QPushButton* cancelButton_;

    //uint fileCountFinished;

    QNetworkReply* reply_;
    
    std::unique_ptr<QFile> file_;
    ResourceCompare Comparer_;
    DownloadState downloadState_;
    bool isBreakPointTranSupported_;
    QNetworkAccessManager QNAManager_;

    QString version_;
    QVector<QString> failed_;
    QVector<QString> filesSkiped_;
    QVector<QString> filesFinished_;
    QVector<QString> filesUnModified_;
    QVector<QString> filesToDownload_;
};



