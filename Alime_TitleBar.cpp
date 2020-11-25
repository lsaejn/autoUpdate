#include <QLabel>
#include <QEvent>

#include <QApplication>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMouseEvent>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#pragma comment(lib, "user32.lib")
#endif

#include "Alime_TitleBar.h"
#include "AlimeQLib/QtAlimeImageButton.h"

namespace 
{
    void InitPushButton(QPushButton* btn, const QString& fileUrl,
        const QString& tooltip, const QString& objName,
        const size_t btnSize, const size_t imageSize, bool show = true)
    {
        btn->setObjectName(objName);
        btn->setFixedSize(btnSize, btnSize);
        btn->setIconSize(QSize(imageSize, imageSize));
        show ? btn->show() : btn->hide();
        btn->setIcon(QIcon(fileUrl));
        btn->setToolTip(tooltip);
    }
}



Alime_TitleBar::Alime_TitleBar(QWidget* parent)
    : QWidget(parent),
    IsNcPressing_(false)
{
    setAttribute(Qt::WA_StyledBackground, true);

    iconLabel_ = new QLabel();
    titleLabel_ = new QLabel(this);
    closeButton_ = new QPushButton(this);
    minimizeButton_ = new QPushButton(this);
    maximizeButton_ = new QPushButton(this);

    iconLabel_->setFixedSize(32, 32);
    iconLabel_->setObjectName("titleLabel");
    iconLabel_->setScaledContents(true);

    titleLabel_->setObjectName("titleLabel");
    titleLabel_->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

    InitPushButton();

    QHBoxLayout* pLayout = new QHBoxLayout(this);

#ifdef ALIME_DEVELOP
    //开发版本提供按钮波纹，点击波纹，定制对话框
    btn_ = new QtAlimeImageButton(QIcon(":/images/PkpmV52.ico"), this);
    btn_->setColor(QColor(53, 99, 203));
    btn_->setIconSize({30, 30});
    btn_->setFixedSize(height(), height());
    pLayout->addWidget(btn_);
    iconLabel_->setVisible(false);
#else
    pLayout->addWidget(iconLabel_);
#endif

    pLayout->addSpacing(10);
    pLayout->addWidget(titleLabel_);
    pLayout->addStretch();
    pLayout->addWidget(minimizeButton_);
    pLayout->addWidget(maximizeButton_);
    pLayout->addWidget(closeButton_);
    pLayout->setSpacing(10);
    pLayout->setContentsMargins(5, 0, 5, 0);
}

void Alime_TitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    emit maximizeButton_->clicked();
}

void Alime_TitleBar::mousePressEvent(QMouseEvent* event)
{
#ifdef Q_OS_WIN
    if (ReleaseCapture() && event->button()== Qt::LeftButton)
    {
        QWidget* pWindow = this->window();
        if (pWindow->isMaximized())
            IsNcPressing_ = true;
        if (pWindow->isTopLevel())
        {
            SendMessage(HWND(pWindow->winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
        }
    }
    event->ignore();
#else
#endif
}


/*
这么处理是因为一个bug，完美版本请移步本人github
*/
void Alime_TitleBar::mouseReleaseEvent(QMouseEvent* event)
{
#ifdef Q_OS_WIN
    if (IsNcPressing_ )
    {
        if (normalStateCallback_)
            normalStateCallback_();
        auto hwnd = (HWND)window()->winId();
        RECT rc;
        ::GetWindowRect(hwnd, &rc);
        MoveWindow(hwnd, rc.left - 5, rc.top - 5, rc.right - rc.left - 10, rc.bottom - rc.top - 10, 0);
        event->ignore();
    }
    IsNcPressing_ = false;
#else
#endif
}

bool Alime_TitleBar::eventFilter(QObject* obj, QEvent* event)
{
    switch (event->type())
    {
    case QEvent::WindowTitleChange:
    {
        QWidget* pWidget = qobject_cast<QWidget*>(obj);
        if (pWidget)
        {
            QString titleName = pWidget->windowTitle();
            titleLabel_->setText(titleName);
            return true;
        }
    }
    case QEvent::WindowIconChange:
    {
        QWidget* pWidget = qobject_cast<QWidget*>(obj);
        if (pWidget)
        {
            QIcon icon = pWidget->windowIcon();
            iconLabel_->setPixmap(icon.pixmap(iconLabel_->size()));
            return false;
        }
    }
    case QEvent::WindowStateChange:
    case QEvent::Resize:
        updateMaximize();
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void Alime_TitleBar::onClicked()
{
    QPushButton* pButton = qobject_cast<QPushButton*>(sender());
    QWidget* pWindow = this->window();
    auto name = pButton->objectName();
    if (pWindow->isTopLevel())
    {
        if (pButton->objectName()=="minimizeButton")
        {
            pWindow->showMinimized();
        }
        else if (pButton->objectName() == "maximizeButton" || pButton->objectName() == "resetButton")
        {
            if (maximizeButton_->objectName()=="maximizeButton" && maxStateCallback_)
            {
                maxStateCallback_();
            }
            else if(normalStateCallback_)
            {
                normalStateCallback_();
            }
            pWindow->isMaximized() ? pWindow->showNormal() : pWindow->showMaximized();
            repaint();
        }
        else if (pButton->objectName() == "closeButton")
        {
            pWindow->close();
        }
    }
}

void Alime_TitleBar::updateMaximize()
{
    QWidget* pWindow = this->window();
    if (pWindow->isTopLevel())
    {
        bool bMaximize = pWindow->isMaximized();
        if (bMaximize)
        {
            maximizeButton_->setIcon(QIcon(":/images/reset.png"));
            maximizeButton_->setObjectName("resetButton");
            maximizeButton_->setToolTip(u8"向下还原");
        }
        else
        {
            maximizeButton_->setIcon(QIcon(":/images/max.png"));
            maximizeButton_->setObjectName("maximizeButton");
            maximizeButton_->setToolTip(u8"最大化");
        }
    }
}

void Alime_TitleBar::InitPushButton()
{
    ::InitPushButton(minimizeButton_, ":/images/min.png", u8"最小化", "minimizeButton", 40, 24);
    ::InitPushButton(maximizeButton_, ":/images/max.png", u8"最大化", "maximizeButton", 40, 24);
    ::InitPushButton(closeButton_, ":/images/close.png", u8"关闭", "closeButton", 40, 24);

    connect(minimizeButton_, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
    connect(maximizeButton_, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
    connect(closeButton_, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
}

bool Alime_TitleBar::SysButtonEventRegister(WindowStateChangeCallback f, bool IsMaxstate)
{
    if (IsMaxstate)
        maxStateCallback_ = f;
    else
        normalStateCallback_ = f;
    return true;
}
