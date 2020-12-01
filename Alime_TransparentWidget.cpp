#include "Alime_TransparentWidget.h"
#include "Alime_WindowBase.h"
#include <QLayout>
#include <QGraphicsDropShadowEffect>
#include <windows.h>
#include <windowsx.h>
#include <QIcon>

Alime_TransparentWidget::Alime_TransparentWidget(QWidget* parent)
	:QWidget(parent),
    boundaryWidth_(5)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    //setWindowIcon(QIcon(":/images/PkpmV52.ico"));
    //ÒõÓ°
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setObjectName("border");
    shadow->setOffset(0, 0);
    shadow->setColor(QColor("#FF444444"));

    shadow->setBlurRadius(10);
    QVBoxLayout* box = new QVBoxLayout(this);
    Alime_WindowBase* base= new Alime_WindowBase(this, box);
    
    box->addWidget(base);

    base->setGraphicsEffect(shadow);
    box->setMargin(5);
}

bool Alime_TransparentWidget::nativeEvent(const QByteArray& /*eventType*/, void* message, long* result)
{
    MSG* msg = (MSG*)message;
    const int boundaryWidth = boundaryWidth_;
    switch (msg->message)
    {
    case WM_NCHITTEST:
        int xPos = GET_X_LPARAM(msg->lParam) - this->frameGeometry().x();
        int yPos = GET_Y_LPARAM(msg->lParam) - this->frameGeometry().y();
        if (xPos < boundaryWidth && yPos < boundaryWidth)
            *result = HTTOPLEFT;
        else if (xPos >= width() - boundaryWidth && yPos < boundaryWidth)
            *result = HTTOPRIGHT;
        else if (xPos < boundaryWidth && yPos >= height() - boundaryWidth)
            *result = HTBOTTOMLEFT;
        else if (xPos >= width() - boundaryWidth && yPos >= height() - boundaryWidth)
            *result = HTBOTTOMRIGHT;
        else if (xPos < boundaryWidth)
            *result = HTLEFT;
        else if (xPos >= width() - boundaryWidth)
            *result = HTRIGHT;
        else if (yPos < boundaryWidth)
            *result = HTTOP;
        else if (yPos >= height() - boundaryWidth)
            *result = HTBOTTOM;
        else
            return false;
        return true;
    }
    return false;
}