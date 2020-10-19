#include <QPainter>
#include <QEvent>
#include <QMouseEvent>

#include "QtAlimeStyle.h"
#include "QtAlimeImageButton.h"
#include "QtAlimeRippleOverlay.h"
#include "details/QtAlimeImageButtonPrivate.h"

QtAlimeImageButton::QtAlimeImageButton(const QIcon& icon, QWidget* parent)
    : QAbstractButton(parent),
    d_ptr(new QtAlimeImageButtonPrivate(this))
{
    d_func()->init();
    setIcon(icon);
}

QtAlimeImageButton::~QtAlimeImageButton()
{
}

QSize QtAlimeImageButton::sizeHint() const
{
    return iconSize();
}

void QtAlimeImageButton::setUseThemeColors(bool value)
{
    Q_D(QtAlimeImageButton);

    if (d->useThemeColors == value)
    {
        return;
    }

    d->useThemeColors = value;
    update();
}

bool QtAlimeImageButton::useThemeColors() const
{
    Q_D(const QtAlimeImageButton);
    return d->useThemeColors;
}

void QtAlimeImageButton::setColor(const QColor& color)
{
    Q_D(QtAlimeImageButton);

    d->color = color;

    ALIME_DISABLE_THEME_COLORS
    update();
}

QColor QtAlimeImageButton::color() const
{
    Q_D(const QtAlimeImageButton);

    if (d->useThemeColors || !d->color.isValid())
    {
        return QtAlimeStyle::instance().themeColor("text");
    }
    return d->color;
}

void QtAlimeImageButton::setDisabledColor(const QColor& color)
{
    Q_D(QtAlimeImageButton);

    d->disabledColor = color;

    ALIME_DISABLE_THEME_COLORS
        update();
}

QColor QtAlimeImageButton::disabledColor() const
{
    Q_D(const QtAlimeImageButton);

    if (d->useThemeColors || !d->disabledColor.isValid()) {
        return QtAlimeStyle::instance().themeColor("disabled");
    }
    return d->disabledColor;
}

QtAlimeImageButton::QtAlimeImageButton(QtAlimeImageButtonPrivate& d, QWidget* parent)
    : QAbstractButton(parent),
    d_ptr(&d)
{
    d_func()->init();
}

bool QtAlimeImageButton::event(QEvent* event)
{
    Q_D(QtAlimeImageButton);

    switch (event->type())
    {
    case QEvent::Move:
    case QEvent::Resize:
        d->updateRipple();
        break;
    case QEvent::ParentChange: {
        QWidget* widget;
        if ((widget = parentWidget())) {
            d->rippleOverlay->setParent(widget);
        }
        break;
    }
    default:
        break;
    }
    return QAbstractButton::event(event);
}

bool QtAlimeImageButton::eventFilter(QObject* obj, QEvent* event)
{
    if (QEvent::Resize == event->type())
    {
        Q_D(QtAlimeImageButton);

        d->updateRipple();
    }
    return QAbstractButton::eventFilter(obj, event);
}

void QtAlimeImageButton::mousePressEvent(QMouseEvent* event)
{
    Q_D(QtAlimeImageButton);

    QPointF pos = event->localPos();
    d->rippleOverlay->addRipple(QPoint(pos.x(), pos.y()),
    iconSize().width()/2);
    emit clicked();

    QAbstractButton::mousePressEvent(event);
}

void QtAlimeImageButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);

    QPixmap pixmap = icon().pixmap(iconSize());
    QPainter icon(&pixmap);
    icon.setCompositionMode(QPainter::CompositionMode_SourceIn);
    icon.fillRect(pixmap.rect(), isEnabled() ? color() : disabledColor());

    QRect r(rect());
    const qreal w = pixmap.width();
    const qreal h = pixmap.height();
    //image{ anchors.fill: parent}
    QRect rc((r.width() - w) / 2, (r.height() - h) / 2, w, h);
    painter.drawPixmap(rc, pixmap);
}
