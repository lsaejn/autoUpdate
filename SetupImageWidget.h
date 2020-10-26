#pragma once
#include <QListWidget>
#include <atomic>
//fix me
//这个QListWidget也没时间写了
//重设QListWidget，以便预设一些样式
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
    std::atomic<bool> isAutoSetupRunning_;//我们要打开优化
};
