#include <QFontDatabase>
#include "QtAlimeStyle.h"
#include "details/QtAlimeStyleDetail.h"
#include "QtAlimeTheme.h"

/*!
 *  \class QtMaterialStylePrivate
 *  \internal
 */

QtAlimeStylePrivate::QtAlimeStylePrivate(QtAlimeStyle* q)
    : q_ptr(q)
{
}

QtAlimeStylePrivate::~QtAlimeStylePrivate()
{
}

void QtAlimeStylePrivate::init()
{
    Q_Q(QtAlimeStyle);

    QFontDatabase::addApplicationFont(":/fonts/roboto_regular");
    QFontDatabase::addApplicationFont(":/fonts/roboto_medium");
    QFontDatabase::addApplicationFont(":/fonts/roboto_bold");

    q->setTheme(new QtAlimeTheme);
}

/*!
 *  \class QtMaterialStyle
 *  \internal
 */

void QtAlimeStyle::setTheme(QtAlimeTheme* theme)
{
    Q_D(QtAlimeStyle);

    d->theme = theme;
    theme->setParent(this);
}

QColor QtAlimeStyle::themeColor(const QString& key) const
{
    Q_D(const QtAlimeStyle);

    Q_ASSERT(d->theme);

    return d->theme->getColor(key);
}

QtAlimeStyle::QtAlimeStyle()
    : QCommonStyle(),
    d_ptr(new QtAlimeStylePrivate(this))
{
    d_func()->init();
}
