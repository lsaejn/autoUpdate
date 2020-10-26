#pragma once
#include "Md5Checker.h"
#include <QtGlobal>
#include <QVector>
#include <QObject>
#include <QHash>

//fix me, use AlimeProgressBar
#include <QProgressDialog>

//class ProgressBarDialog: public QWidget
//{
//public:
//    ProgressBarDialog() = default;
//    ~ProgressBarDialog() = default;
//public slots:
//    void 
//};




class Parser: public QObject
{
public slots:
    void onFinished();
    void onErrorOccurred();
public:
    virtual bool Parse()=0;
    virtual bool GetFileContent() = 0;
public:
    QString fileContent;
};

class ResourceCompare
{
public:
    using FileVec = QVector<QString>;

    ResourceCompare();
    ~ResourceCompare() = default;
    //set
    void Init();
    bool Scan();
    QString RootPath();
    void CreateCompareResult();
    void SetRootPath(const QString& localPath);
    void SetFileVector(FileVec*, FileVec*, FileVec*);
    void SetTargetVersion(const QString& version);
    void WriteFile(const QString& filePath);
    
    Q_DISABLE_COPY(ResourceCompare)
private:
    //ini file
    class PkpmMD5File: public Parser
    {
    public:
        bool Parse();
        bool GetFileContent();
    public: 
        QHash<QString, QString> md5Map;
    };

private:
    void ShowProgressBar();
    void UpdateProgressBar(int done, int total);
private slots:
    void DestoryProgressBar();

private:
    bool isStateGood_;
    QString rootPath_;
    FileVec* filesSkiped_;
    FileVec* filesUnModified_;
    FileVec* filesToDownload_;
    PkpmMD5File parser_;
    MD5FolderScanner scanner_;
    QProgressDialog* dialog_;
};

