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
��ҪStateManager+FileManager
������ؼ�
downloadState_ 
downloadStatusLabel_
*/

/*
û�н����غ�widget���룬���º���ά���е�����
������µĻ���Ҫ��дһ����
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

    void finishSetup();//ʹ��ѹ������Ľ���������ǲ�֪���ǲ��ǰ�װ�ɹ���������֪ͨQlistWidget

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
        Iso =1,//����
        Zip=2,//zip
        Other
    };

    //�û�����Paused / Cancel
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
    /// ��ʼ���ز���װ
    /// </summary>
    /// <returns>�Ƿ�ɹ���������״̬</returns>
    bool StartDownloadTask();

    /// <summary>
    /// ֹͣ����
    /// </summary>
    /// <returns>״̬�Ƿ����߼�����</returns>
    bool PauseDownloadTask();

    /// <summary>
    /// �˳����ء�������״̬��ɾ���ļ���������Cancel
    /// </summary>
    /// <returns></returns>
    bool CancelDownloadTask();

    /// <summary>
    /// ��ʼ��װ
    /// </summary>
    /// <returns>�Ƿ���״̬����</returns>
    bool DoSetup();

    /// <summary>
    ///  �жϰ�װ������
    /// </summary>
    /// <returns>PackType</returns>
    PackType GetPackType();

    /// <summary>
    /// ���ؽ���
    /// </summary>
    /// <returns></returns>
    bool IsFinished();//download

    /// <summary>
    /// �ж��Ƿ���ִ�а�װ��������̺ͽ�����ܱ�������ơ�
    /// </summary>
    /// <returns>bool</returns>
    bool IsSetuping();

    /// <summary>
    /// �Ƿ�������״̬
    /// </summary>
    /// <returns></returns>
    bool IsDownLoading();

    /// <summary>
    /// ����
    /// </summary>
    /// <param name="f"></param>
    void SetCheckCallBack(CheckCallBack f);

    /// <summary>
    /// ����
    /// </summary>
    /// <param name="isUpdatePackage"></param>
    void SetPackFlag(PackType ty);
private:

    /// <summary>
    /// ������ֹ�Ĳۺ���
    /// </summary>
    void httpFinished();

    /// <summary>
    /// ���ݻص�����
    /// </summary>
    void httpReadyRead();

    /// <summary>
    /// �����Ҽ��˵�
    /// </summary>
    void AddMenuItems();
    
    /// <summary>
    /// ���������밲װ״̬����װ���տ��ƣ�����޷���ʾ�ٷֱ�
    /// </summary>
    /// <param name=""></param>
    void ShowSetupProgress(bool);

    /// <summary>
    /// ����
    /// </summary>
    void UpdateUiAccordingToState();

    /// <summary>
    /// ����
    /// </summary>
    bool CheckVersionFileAfterSetup();

    /// <summary>
    /// ������������ʼ����
    /// </summary>
    /// <param name="url"></param>
    void StartRequest(const QUrl& url);

    /// <summary>
    /// ���¿ؼ��ļ��
    /// </summary>
    /// <param name="second"></param>
    /// <returns></returns>
    bool isTimeToUpdate(double& second);

    /// <summary>
    /// ׼������
    /// </summary>
    /// <param name=""></param>
    void httpError(QNetworkReply::NetworkError);

    /// <summary>
    /// ƴ�����ش�С�����ַ���
    /// </summary>
    /// <returns></returns>
    QString MakeDownloadHeadway();
    QString MakeDownloadHeadway(int64_t reader, int64_t total);

    /// <summary>
    /// ת����׼DateTime
    /// </summary>
    /// <param name="seconds"></param>
    /// <returns>DateTime</returns>
    QString MakeDurationToString(int seconds);

    /// <summary>
    /// ���ؽ�����
    /// </summary>
    void LoadingProgressForBreakPoint();

    /// <summary>
    /// ���ݽ��ȸ��¿ؼ�
    /// </summary>
    /// <param name="bytesReceived"></param>
    /// <param name="bytesTotal"></param>
    void UpdateChildWidgets(qint64 bytesReceived, qint64 bytesTotal);

    /// <summary>
    /// ׼����������ʼ/��ͣ״̬��ʾ��ͬ�İ�ť
    /// </summary>
    /// <param name="stopped"></param>
    void UpdatePlayButton(bool stopped=true);

    /// <summary>
    /// ������������һ���ļ�
    /// </summary>
    /// <param name="fileName">�ļ���</param>
    /// <returns>���</returns>
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

    static std::atomic_bool Setuping_;
    PackType packType_;
    bool isBreakPointTranSupported_;
};


