#pragma once
#include <QListWidget>
#include <atomic>
//fix me
//���QListWidgetҲûʱ��д��
//����QListWidget���Ա�Ԥ��һЩ��ʽ
class SetupWidget : public QListWidget
{
    Q_OBJECT
signals:
    void installing(int);
    void finish(int);
    void error();
public:
    SetupWidget(QWidget* parent);
    
    void SetupAllTask();
    bool IsAutoSetupOn();

private:
    //������Ϊ�˷�ֹδ��ά���߿��������Ż�����һ������һ�����һ���߳��������ļ�
    std::atomic<bool> isAutoSetupRunning_;
};
