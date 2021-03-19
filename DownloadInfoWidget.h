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
#include "TypeDefine.h"

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

/*
没有将下载和widget分离，导致后面维护有点困难
差异更新的话需要重写一个类
*/
//Widget as a ListItem in QListWidget
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

    void finishSetup();//使用压包软件的结果就是我们不知道是不是安装成功。但我们通知QlistWidget

public slots:
    void ShowTipsWhenSetupFinished(int);
    void SetupStarted();
    void SetupFinished();
    
public:
    DownloadInfoWidget(QWidget* parent, const QString& fileName,
        qint64 fileSize, const QUrl& url, const QUrl& instructionUrl, PackType ty);
    
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
        Iso =1,//光盘
        Zip=2,//zip
        Other
    };

    //用户发起Paused / Cancel
    enum class DownloadState
    {
        NotStarted,
        Downloading,
        Paused,
        Cancel,
        Finished
    };

public:

    /// <summary>
    /// 开始下载并安装
    /// </summary>
    /// <returns>是否成功进入下载状态</returns>
    bool StartDownloadTask();

    /// <summary>
    /// 停止下载
    /// </summary>
    /// <returns>状态是否有逻辑错误</returns>
    bool PauseDownloadTask();

    /// <summary>
    /// 退出下载。非下载状态将删除文件，并进入Cancel
    /// </summary>
    /// <returns></returns>
    bool CancelDownloadTask();

    /// <summary>
    /// 开始安装
    /// </summary>
    /// <returns>是否有状态错误</returns>
    bool DoSetup();

    /// <summary>
    ///  判断安装包类型
    /// </summary>
    /// <returns>PackType</returns>
    PackType GetPackType();

    /// <summary>
    /// 下载结束
    /// </summary>
    /// <returns></returns>
    bool IsFinished();//download

    /// <summary>
    /// 判断是否在执行安装。这个过程和结果不受本程序控制。
    /// </summary>
    /// <returns>bool</returns>
    bool IsSetuping();

    /// <summary>
    /// 是否处于下载状态
    /// </summary>
    /// <returns></returns>
    bool IsDownLoading();

    /// <summary>
    /// 废弃
    /// </summary>
    /// <param name="f"></param>
    void SetCheckCallBack(CheckCallBack f);

    /// <summary>
    /// 废弃
    /// </summary>
    /// <param name="isUpdatePackage"></param>
    void SetPackFlag(PackType ty);
private:

    /// <summary>
    /// 下载中止的槽函数
    /// </summary>
    void httpFinished();

    /// <summary>
    /// 数据回调函数
    /// </summary>
    void httpReadyRead();

    /// <summary>
    /// 增加右键菜单
    /// </summary>
    void AddMenuItems();
    
    /// <summary>
    /// 进度条进入安装状态。安装不收控制，因此无法显示百分比
    /// </summary>
    /// <param name=""></param>
    void ShowSetupProgress(bool);

    /// <summary>
    /// 废弃
    /// </summary>
    void UpdateUiAccordingToState();

    /// <summary>
    /// 废弃
    /// </summary>
    bool CheckVersionFileAfterSetup();

    /// <summary>
    /// 辅助函数。开始请求。
    /// </summary>
    /// <param name="url"></param>
    void StartRequest(const QUrl& url);

    /// <summary>
    /// 更新控件的间隔
    /// </summary>
    /// <param name="second"></param>
    /// <returns></returns>
    bool isTimeToUpdate(double& second);

    /// <summary>
    /// 准备废弃
    /// </summary>
    /// <param name=""></param>
    void httpError(QNetworkReply::NetworkError);

    /// <summary>
    /// 拼接下载大小进度字符串
    /// </summary>
    /// <returns></returns>
    QString MakeDownloadHeadway();
    QString MakeDownloadHeadway(int64_t reader, int64_t total);

    /// <summary>
    /// 转换标准DateTime
    /// </summary>
    /// <param name="seconds"></param>
    /// <returns>DateTime</returns>
    QString MakeDurationToString(int seconds);

    /// <summary>
    /// 加载进度条
    /// </summary>
    void LoadingProgressForBreakPoint();

    /// <summary>
    /// 根据进度更新控件
    /// </summary>
    /// <param name="bytesReceived"></param>
    /// <param name="bytesTotal"></param>
    void UpdateChildWidgets(qint64 bytesReceived, qint64 bytesTotal);

    /// <summary>
    /// 准备废弃。开始/暂停状态显示不同的按钮
    /// </summary>
    /// <param name="stopped"></param>
    void UpdatePlayButton(bool stopped=true);

    /// <summary>
    /// 辅助函数。打开一个文件
    /// </summary>
    /// <param name="fileName">文件名</param>
    /// <returns>句柄</returns>
    std::unique_ptr<QFile> openFileForWrite(const QString& fileName);
    
private:
    QUrl url_;
    QUrl instructionUrl_;
    QString fileName_;
    QString localFilePath_;

    QLabel* fileNameLabel_;//文件名
    QLabel* leftTimeEstimated_;
    QLabel* downloadStatusLabel_;//显示下载/安装状态
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
    QNetworkAccessManager QNAManager_;

    static std::atomic_bool Setuping_;
    PackType packType_;
    bool isBreakPointTranSupported_;
};


