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
///�ðɣ��������·ݣ�PackageListWidget����������/���� ������
///��ΪҪ��һ��ҳ����ʾ��
/// 

class DownloadInfoWidget;
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

    DownloadInfoWidget* AddItem(QWidget* _parent,
        qint64 _fileSize, const QUrl& _url, const QString& filename);
    
    void SetupAllTask();


    void Parse(const nlohmann::json& json_);
    void PackageListWidget::GetUpdatePackUrl(const nlohmann::json& json,
        std::string& urlOut, bool& showImage);

    void ReadUpdatePack(const nlohmann::json& json_);
    void ReadFixPack(const nlohmann::json& json_);
    
    //bool IsAutoSetupOn();

    //��widget���Ƿ���item��������
    bool HasSetupItem();

    void SetVersion(const std::string& mainV, const std::string& localV);
private:
    void ReadSetupImage(const nlohmann::json& json_);
    //������Ϊ�˷�ֹδ��ά���߿��������Ż�����һ������һ�����һ���߳��������ļ�
    //std::atomic<bool> isAutoSetupRunning_;
    std::string mainVersionLocal_; //Vxy����ʷԭ�� �����ж�ע����ֵ
    std::string versionLocal_;
};


//��״̬ģʽ����õĴ������Ǵ���Ҫ���
class IState
{

};
