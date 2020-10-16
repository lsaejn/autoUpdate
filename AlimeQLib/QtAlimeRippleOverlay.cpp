#include <QPainter>

#include "QtAlimeRippleOverlay.h"
#include "QtAlimeRipple.h"


QtAlimeRippleOverlay::QtAlimeRippleOverlay(QWidget *parent)
    : QtAlimeOverlayWidget(parent),
      m_useClip(false)
{
    //rippleWidget不关心任何事件, 只处理ripples
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
}

QtAlimeRippleOverlay::~QtAlimeRippleOverlay()
{
}

void QtAlimeRippleOverlay::addRipple(QtAlimeRipple *ripple)
{
    ripple->setOverlay(this);
    m_ripples.push_back(ripple);
    ripple->start();

    connect(this, SIGNAL(destroyed(QObject*)), ripple, SLOT(stop()));
    connect(this, SIGNAL(destroyed(QObject*)), ripple, SLOT(deleteLater()));
}

void QtAlimeRippleOverlay::addRipple(const QPoint &position, qreal radius)
{
    QtAlimeRipple *ripple = new QtAlimeRipple(position);
    ripple->setRadiusEndValue(radius);
    addRipple(ripple);
}

void QtAlimeRippleOverlay::removeRipple(QtAlimeRipple *ripple)
{
    if (m_ripples.removeOne(ripple)) {
        delete ripple;
        update();
    }
}

void QtAlimeRippleOverlay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    if (m_useClip) {
        painter.setClipPath(m_clipPath);
    }

    QList<QtAlimeRipple*>::const_iterator i;
    for (i = m_ripples.begin(); i != m_ripples.end(); ++i) {
        paintRipple(&painter, *i);
    }
}

void QtAlimeRippleOverlay::paintRipple(QPainter *painter, QtAlimeRipple *ripple)
{
    const qreal radius = ripple->radius();
    const QPointF center = ripple->center();
    painter->setOpacity(ripple->opacity());
    painter->setBrush(ripple->brush());
    painter->drawEllipse(center, radius, radius);
}

void QtAlimeRippleOverlay::setClipping(bool enable)
{
    m_useClip = enable;
    update();
}

bool QtAlimeRippleOverlay::hasClipping() const
{
    return m_useClip;
}

void QtAlimeRippleOverlay::setClipPath(const QPainterPath& path)
{
    m_clipPath = path;
    update();
}

QList<QtAlimeRipple*> QtAlimeRippleOverlay::ripples() const
{
    return m_ripples;
}
