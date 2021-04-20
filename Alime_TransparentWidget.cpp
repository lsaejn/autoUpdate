#include "Alime_TransparentWidget.h"
#include "Alime_WindowBase.h"
#include <QLayout>
#include <QGraphicsDropShadowEffect>
#include <windows.h>
#include <windowsx.h>
#include <QIcon>
#include <QScreen>
#include <QGuiApplication>

Alime_TransparentWidget::Alime_TransparentWidget(QWidget* parent)
	:QWidget(parent),
    boundaryWidth_(5)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setObjectName("border");
    shadow->setOffset(0, 0);
    shadow->setColor(QColor("#FF444444"));
    shadow->setBlurRadius(10);

    QVBoxLayout* box = new QVBoxLayout(this);
    Alime_WindowBase* base= new Alime_WindowBase(this, box);//阴影border宽度是从content类里获取
    
    box->addWidget(base);
    

    //阴影加在base上
    base->setGraphicsEffect(shadow);
}

bool Alime_TransparentWidget::nativeEvent(const QByteArray& /*eventType*/, void* message, long* result)
{
    MSG* msg = (MSG*)message;
    int boundaryWidth = boundaryWidth_;
    HDC desktopDc = GetDC(NULL);
    // Get native resolution
    float horizontalDPI = GetDeviceCaps(desktopDc, LOGPIXELSX);
    float verticalDPI = GetDeviceCaps(desktopDc, LOGPIXELSY);
    double dpi = (horizontalDPI + verticalDPI) / 2/96;

    boundaryWidth *= dpi;
    //auto gxy = this->mapToParent({ left, top });
    switch (msg->message)
    {
    case WM_NCHITTEST:
        QPoint qp = QCursor::pos();
        int xPos = qp.x() - this->frameGeometry().x();
        int yPos = qp.y() - this->frameGeometry().y();
        
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