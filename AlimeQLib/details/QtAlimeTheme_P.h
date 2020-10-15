#ifndef QTMATERIALTHEME_P_H
#define QTMATERIALTHEME_P_H

#include <QHash>
#include <QColor>

class QtAlimeTheme;

class QtAlimeThemePrivate
{
    Q_DISABLE_COPY(QtAlimeThemePrivate)
    Q_DECLARE_PUBLIC(QtAlimeTheme)

public:
    QtAlimeThemePrivate(QtAlimeTheme *q);
    ~QtAlimeThemePrivate();

    QColor rgba(int r, int g, int b, qreal a) const;

    QtAlimeTheme *const q_ptr;
    QHash<QString, QColor> colors;
};

#endif // QTMATERIALTHEME_P_H
