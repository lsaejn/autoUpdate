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
    //仅仅是为了防止未来维护者开启代码优化。下一版代码我会在另一个线程里下载文件
    std::atomic<bool> isAutoSetupRunning_;
};
