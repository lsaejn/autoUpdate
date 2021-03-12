#pragma once
#include <QListWidget>
#include <atomic>
//fix me

/*
*需求突然修改导致以下问题:
*1.业务上不能再同时更新两个StackWidget里的item了，否则文件会错乱。
* 因此，当一键更新时，我禁止了切换StackWidget的切换，
* 又因此，一个更新按钮就可以了。StackWidget只塞SetupWidget
*2.一键更新被打断的方式太多了
* 无论如何，SetupWidget里的自定义widget和更新按钮都不能做绑定，
* 因为用户随时可以操作按钮。
* 因此，当一键更新运行时，我直接禁止了用户操作以便视这次更新为一次完整事务。
* 这样，用最简单的方式解决了交互问题。
*/
//而SetupWidget和更新按钮是同级的，这导致了一些交互问题
//现在SetupWidget
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

    //本widget下是否有item正在下载
    bool HasSetupItem();

private:
    //仅仅是为了防止未来维护者开启代码优化。下一版代码我会在另一个线程里下载文件
    std::atomic<bool> isAutoSetupRunning_;
};


//用状态模式是最好的处理，但是代码要大改
class IState
{

};
