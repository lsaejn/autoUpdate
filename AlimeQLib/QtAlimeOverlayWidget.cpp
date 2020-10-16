#include "QtAlimeOverlayWidget.h"
#include <QEvent>


QtAlimeOverlayWidget::QtAlimeOverlayWidget(QWidget *parent)
    : QWidget(parent)
{
    //捕获parent消息
    if (parent) {
        parent->installEventFilter(this);
    }
}

QtAlimeOverlayWidget::~QtAlimeOverlayWidget()
{
}

bool QtAlimeOverlayWidget::event(QEvent *event)
{
    if (!parent()) {
        return QWidget::event(event);
    }
    switch (event->type())
    {
    case QEvent::ParentChange:
    {
        parent()->installEventFilter(this);
        setGeometry(overlayGeometry());
        break;
    }
    case QEvent::ParentAboutToChange:
    {
        parent()->removeEventFilter(this);
        break;
    }
    default:
        break;
    }
    return QWidget::event(event);
}

bool QtAlimeOverlayWidget::eventFilter(QObject *obj, QEvent *event)
{
    //保证本Widget大小和parent一致
    switch (event->type())
    {
    case QEvent::Move:
    case QEvent::Resize:
        setGeometry(overlayGeometry());
        break;
    default:
        break;
    }
    return QWidget::eventFilter(obj, event);
}

QRect QtAlimeOverlayWidget::overlayGeometry() const
{
    QWidget *widget = parentWidget();
    if (!widget) {
        return QRect();
    }
    return widget->rect();
}
