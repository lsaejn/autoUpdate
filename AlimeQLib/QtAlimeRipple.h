#pragma once

#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <QPropertyAnimation>
#include <QPoint>
#include <QBrush>

class QtAlimeRippleOverlay;

class QtAlimeRipple : public QParallelAnimationGroup
{
    Q_OBJECT

    Q_PROPERTY(qreal radius WRITE setRadius READ radius)
    Q_PROPERTY(qreal opacity WRITE setOpacity READ opacity)

public:
    explicit QtAlimeRipple(const QPoint &center, QObject *parent = 0);
    QtAlimeRipple(const QPoint &center, QtAlimeRippleOverlay *overlay, QObject *parent = 0);
    ~QtAlimeRipple();

    qreal radius() const;
    qreal opacity() const;
    QColor color() const;
    QBrush brush() const;
    QPoint center() const;

    void setDuration(int msecs);
    void setRadius(qreal radius);
    void setOpacity(qreal opacity);
    void setColor(const QColor& color);
    void setBrush(const QBrush& brush);
    void setRadiusEndValue(qreal value);
    void setRadiusStartValue(qreal value);
    void setOpacityEndValue(qreal value);
    void setOpacityStartValue(qreal value);
    void setOverlay(QtAlimeRippleOverlay* overlay);
    QPropertyAnimation *radiusAnimation() const;
    QPropertyAnimation *opacityAnimation() const;

protected slots:
    void destroy();

private:
    Q_DISABLE_COPY(QtAlimeRipple)
        
    void init();
    QPropertyAnimation *animate(const QByteArray &property,
                                const QEasingCurve &easing = QEasingCurve::OutQuad,
                                int duration = 800);
private:
    qreal   m_radius;
    qreal   m_opacity;
    QPoint  m_center;
    QBrush  m_brush;
    QtAlimeRippleOverlay* m_overlay;
    QPropertyAnimation* const m_radiusAnimation;
    QPropertyAnimation* const m_opacityAnimation;
};
