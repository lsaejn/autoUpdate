#pragma once
#include <QListWidget>
#include <atomic>

#include "thirdParty/nlohmann/json.hpp"
//fix me

/*
*����ͻȻ�޸ĵ�����������:
*1.ҵ���ϲ�����ͬʱ��������StackWidget���item�ˣ������ļ�����ҡ�
* ��ˣ���һ������ʱ���ҽ�ֹ���л�StackWidget���л���
* ����ˣ�һ�����°�ť�Ϳ����ˡ�StackWidgetֻ��SetupWidget
*2.һ�����±���ϵķ�ʽ̫����
* ������Σ�SetupWidget����Զ���widget�͸��°�ť���������󶨣�
* ��Ϊ�û���ʱ���Բ�����ť��
* ��ˣ���һ����������ʱ����ֱ�ӽ�ֹ���û������Ա�����θ���Ϊһ����������
* ����������򵥵ķ�ʽ����˽������⡣
*/
//��SetupWidget�͸��°�ť��ͬ���ģ��⵼����һЩ��������
//����SetupWidget
///�ðɣ��������·ݣ�PackageListWidget����������/����, ������
///Ҫ��һ��ҳ����ʾ��
///������Ϊ��������һ��

class DownloadInfoWidget;
class QFrameLessWidget_Alime;

class PackageListWidget : public QListWidget
{
    Q_OBJECT
signals:
    void installing(int);
    void finish(int);
    void error();
    void reset();

public:
    PackageListWidget(QWidget* parent);

    DownloadInfoWidget* AddItem(QWidget* _parent,qint64 _fileSize, const QUrl& _url, const QUrl& ins_url, const QString& filename,
        bool isUpdatePack=true);
    void SetupAllTask();
    void Parse(const nlohmann::json& json_);
    void GetUpdatePackUrl(const nlohmann::json& json, std::string& urlOut, std::string& instructionUrl, bool& showImage);
    void ReadUpdatePack(const nlohmann::json& json_);
    void ReadFixPack(const nlohmann::json& json_);
    
    //bool IsAutoSetupOn();

    //��widget���Ƿ���item��������
    //������������������ͬʱ��װ
    bool HasSetupItem();

    void SetVersion(const std::string& mainV, const std::string& localV);

    void SetMainWidget(QFrameLessWidget_Alime* p) { mainWidget_ = p; };
private:
    void ReReadPacks(bool isUpdatePack);
    void ReadSetupImage(const nlohmann::json& json_);

    std::string mainVersionLocal_; //Vx.y����ʷԭ�� �����ж�ע����ֵ
    std::string versionLocal_;
    QFrameLessWidget_Alime* mainWidget_;
};
