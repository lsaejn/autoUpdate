#pragma once

#include <QWidget>
#include <functional>

using WindowStateChangeCallback = std::function<void()>;

class QLabel;
class QPushButton;

class Alime_TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit Alime_TitleBar(QWidget* parent = 0);
    ~Alime_TitleBar();
    bool SysButtonEventRegister(WindowStateChangeCallback f, bool IsMaxstate);
    
protected:
    //virtual void paintEvent(QPaintEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    //virtual void mouseMoveEvent(QMouseEvent* event);
    virtual bool eventFilter(QObject* obj, QEvent* event);
private slots:
    void onClicked();
private:
    void updateMaximize();
    void InitPushButton();
private:
    QLabel* iconLabel_;
    QLabel* titleLabel_;
    QPushButton* minimizeButton_;
    QPushButton* maximizeButton_;
    QPushButton* closeButton_;
    WindowStateChangeCallback maxStateCallback_;
    WindowStateChangeCallback normalStateCallback_;
    bool IsNcPressing_;
};