#include <QPainterPath>
#include "QtAlimeOverlayWidget.h"

class QtAlimeRipple;

class QtAlimeRippleOverlay : public QtAlimeOverlayWidget
{
    Q_OBJECT

public:
    explicit QtAlimeRippleOverlay(QWidget *parent = 0);
    ~QtAlimeRippleOverlay();

    void addRipple(QtAlimeRipple*ripple);
    void addRipple(const QPoint &position, qreal radius = 300);
    void removeRipple(QtAlimeRipple *ripple);

    void setClipping(bool enable);
    bool hasClipping() const;

    void setClipPath(const QPainterPath &path);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

    QList<QtAlimeRipple*> ripples() const;

private:
    Q_DISABLE_COPY(QtAlimeRippleOverlay)

    void paintRipple(QPainter *painter, QtAlimeRipple *ripple);

    QList<QtAlimeRipple*> m_ripples;
    QPainterPath              m_clipPath;
    bool                      m_useClip;
};
