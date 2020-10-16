#include "QtAlimeRipple.h"
#include "QtAlimeRippleOverlay.h"


QtAlimeRipple::QtAlimeRipple(const QPoint &center, QObject *parent)
    : QParallelAnimationGroup(parent),
      m_overlay(0),
      m_radiusAnimation(animate("radius")),
      m_opacityAnimation(animate("opacity")),
      m_radius(0),
      m_opacity(0),
      m_center(center)
{
    init();
}

QtAlimeRipple::QtAlimeRipple(const QPoint &center,
    QtAlimeRippleOverlay *overlay, 
    QObject *parent)
    : QParallelAnimationGroup(parent),
      m_overlay(overlay),
      m_radiusAnimation(animate("radius")),
      m_opacityAnimation(animate("opacity")),
      m_radius(0),
      m_opacity(0),
      m_center(center)
{
    init();
}

QtAlimeRipple::~QtAlimeRipple()
{
}

void QtAlimeRipple::setRadius(qreal radius)
{
    Q_ASSERT(m_overlay);

    if (m_radius == radius) {
        return;
    }
    m_radius = radius;
    m_overlay->update();
}

void QtAlimeRipple::setOpacity(qreal opacity)
{
    Q_ASSERT(m_overlay);

    if (m_opacity == opacity) {
        return;
    }
    m_opacity = opacity;
    m_overlay->update();
}

void QtAlimeRipple::setColor(const QColor &color)
{
    if (m_brush.color() == color)
    {
        return;
    }
    m_brush.setColor(color);

    if (m_overlay)
    {
        m_overlay->update();
    }
}

void QtAlimeRipple::setBrush(const QBrush &brush)
{
    m_brush = brush;

    if (m_overlay)
    {
        m_overlay->update();
    }
}

void QtAlimeRipple::destroy()
{
    Q_ASSERT(m_overlay);
    m_overlay->removeRipple(this);
}

QPropertyAnimation *QtAlimeRipple::animate(const QByteArray &property,
    const QEasingCurve &easing,
    int duration)
{
    QPropertyAnimation *animation = new QPropertyAnimation;
    animation->setTargetObject(this);
    animation->setPropertyName(property);
    animation->setEasingCurve(easing);
    animation->setDuration(duration);
    addAnimation(animation);
    return animation;
}

void QtAlimeRipple::init()
{
    setOpacityStartValue(0.5);
    setOpacityEndValue(0);
    setRadiusStartValue(0);
    setRadiusEndValue(300);

    m_brush.setColor(Qt::black);
    m_brush.setStyle(Qt::SolidPattern);

    connect(this, SIGNAL(finished()), this, SLOT(destroy()));
}

void QtAlimeRipple::setOverlay(QtAlimeRippleOverlay* overlay)
{
    m_overlay = overlay;
}

qreal QtAlimeRipple::radius() const
{
    return m_radius;
}

qreal QtAlimeRipple::opacity() const
{
    return m_opacity;
}

QColor QtAlimeRipple::color() const
{
    return m_brush.color();
}

QBrush QtAlimeRipple::brush() const
{
    return m_brush;
}

QPoint QtAlimeRipple::center() const
{
    return m_center;
}

QPropertyAnimation* QtAlimeRipple::radiusAnimation() const
{
    return m_radiusAnimation;
}

QPropertyAnimation* QtAlimeRipple::opacityAnimation() const
{
    return m_opacityAnimation;
}

void QtAlimeRipple::setOpacityStartValue(qreal value)
{
    m_opacityAnimation->setStartValue(value);
}

void QtAlimeRipple::setOpacityEndValue(qreal value)
{
    m_opacityAnimation->setEndValue(value);
}

void QtAlimeRipple::setRadiusStartValue(qreal value)
{
    m_radiusAnimation->setStartValue(value);
}

void QtAlimeRipple::setRadiusEndValue(qreal value)
{
    m_radiusAnimation->setEndValue(value);
}

void QtAlimeRipple::setDuration(int msecs)
{
    m_radiusAnimation->setDuration(msecs);
    m_opacityAnimation->setDuration(msecs);
}
