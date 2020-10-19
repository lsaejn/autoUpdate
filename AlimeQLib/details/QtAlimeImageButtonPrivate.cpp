#include "QtAlimeImageButtonPrivate.h"
#include "../QtAlimeRippleOverlay.h"
#include "../QtAlimeImageButton.h"
#include "../QtAlimeStyle.h"

QtAlimeImageButtonPrivate::QtAlimeImageButtonPrivate(QtAlimeImageButton* q)
    : q_ptr(q)
{
}

QtAlimeImageButtonPrivate::~QtAlimeImageButtonPrivate()
{
}

void QtAlimeImageButtonPrivate::init()
{
    Q_Q(QtAlimeImageButton);

    rippleOverlay = new QtAlimeRippleOverlay(q->parentWidget());
    useThemeColors = true;

    rippleOverlay->installEventFilter(q);

    q->setStyle(&QtAlimeStyle::instance());

    QSizePolicy policy;
    policy.setWidthForHeight(true);
    q->setSizePolicy(policy);
}

void QtAlimeImageButtonPrivate::updateRipple()
{
    Q_Q(QtAlimeImageButton);

    QRect r(q->rect());
    //r.setSize(QSize(q->width() * 2, q->height() * 2));
    r.moveCenter(q->geometry().center());
    rippleOverlay->setGeometry(r);
}