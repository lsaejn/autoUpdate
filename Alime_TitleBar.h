#pragma once

#include <QWidget>

#include <functional>

using WindowStateChangeCallback = std::function<void()>;

class QLabel;
class QPushButton;
class QtAlimeImageButton;

class Alime_TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit Alime_TitleBar(QWidget* parent = 0);
    ~Alime_TitleBar()=default;

    bool SysButtonEventRegister(WindowStateChangeCallback f, bool IsMaxState);

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual bool eventFilter(QObject* obj, QEvent* event);

private slots:
    void onClicked();
private:
    void updateMaximize();
    void InitPushButton();
private:
    
    QLabel* iconLabel_;
    QLabel* titleLabel_;

    QtAlimeImageButton* btn_;

    QPushButton* closeButton_;
    QPushButton* minimizeButton_;
    QPushButton* maximizeButton_;
    
    WindowStateChangeCallback maxStateCallback_;
    WindowStateChangeCallback normalStateCallback_;

    //Qt似乎没有什么完美的无边框阴影窗口解决方案
    //自己改了一个。
    bool IsNcPressing_;
};