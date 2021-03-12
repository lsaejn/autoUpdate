#pragma once
#include <QListWidget>
#include <atomic>
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
    
    void SetupAllTask();
    bool IsAutoSetupOn();

    //��widget���Ƿ���item��������
    bool HasSetupItem();

private:
    //������Ϊ�˷�ֹδ��ά���߿��������Ż�����һ������һ�����һ���߳��������ļ�
    std::atomic<bool> isAutoSetupRunning_;
};


//��״̬ģʽ����õĴ������Ǵ���Ҫ���
class IState
{

};
