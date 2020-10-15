#pragma once

#include <QtWidgets/QCommonStyle>
#include "details/QtAlimeStyle_P.h"

#define ALIME_DISABLE_THEME_COLORS \
    if (d->useThemeColors == true) { d->useThemeColors = false; }

class QtAlimeTheme;

class QtAlimeStyle : public QCommonStyle
{
    Q_OBJECT

public:
    inline static QtAlimeStyle& instance();

    void setTheme(QtAlimeTheme* theme);
    QColor themeColor(const QString& key) const;

protected:
    const QScopedPointer<QtAlimeStylePrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(QtAlimeStyle)

        QtAlimeStyle();

    QtAlimeStyle(QtAlimeStyle const&);
    void operator=(QtAlimeStyle const&);
};

inline QtAlimeStyle& QtAlimeStyle::instance()
{
    static QtAlimeStyle instance;
    return instance;
}
