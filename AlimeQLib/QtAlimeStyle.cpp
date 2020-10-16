#include <QFontDatabase>

#include "QtAlimeStyle.h"
#include "QtAlimeTheme.h"




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
