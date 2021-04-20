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
    Alime_WindowBase* base= new Alime_WindowBase(this, box);//��Ӱborder����Ǵ�content�����ȡ
    
    box->addWidget(base);
    

    //��Ӱ����base��
    base->setGraphicsEffect(shadow);
}

bool Alime_TransparentWidget::nativeEvent(const QByteArray& /*eventType*/, void* message, long* result)
{
    MSG* msg = (MSG*)message;
    int boundaryWidth = boundaryWidth_;
    HDC desktopDc = GetDC(NULL);

    //

    // Get native resolution
    float horizontalDPI = GetDeviceCaps(desktopDc, LOGPIXELSX);
    float verticalDPI = GetDeviceCaps(desktopDc, LOGPIXELSY);
    double dpi = (horizontalDPI + verticalDPI) / 2/96;

    boundaryWidth *= dpi;
    //auto gxy = this->mapToParent({ left, top });
    switch (msg->message)
    {
    case WM_NCHITTEST:
        //��GetCursorPos()�������һ��������Ļ����;
        int xScreen = GET_X_LPARAM(msg->lParam);
        int yScreen = GET_Y_LPARAM(msg->lParam);

        //ʹ��QCursor::pos()������ʡ�ü���dpiScale
        QPoint qp = QCursor::pos();

        QPoint pos = this->mapFromGlobal(qp);

        int xPos = pos.x();
        int yPos = pos.y();
        
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